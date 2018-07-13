#pragma once

#include "Platform/Types.h"
#include "Platform/System.h"

#include "Foundation/API.h"
#include "Foundation/Endian.h"
#include "Foundation/Event.h"

namespace Helium
{

    namespace IPC
    {
        class Message;
        class Connection;
    }

    namespace RPC
    {
        typedef void (*SwizzleFunc)(void* data);

        template <class T>
        inline void Swizzle(T* data)
        {
#ifdef HELIUM_ENDIAN_LITTLE
            Helium::Swizzle(*data, true);
#endif
        }

        template <class T>
        inline void Swizzle(T& data)
        {
#ifdef HELIUM_ENDIAN_LITTLE
            Swizzle(&data);
#endif
        }

        template <class T>
        SwizzleFunc GetSwizzleFunc()
        {
            void (*func)(T*) = &Swizzle<T>;
            return (SwizzleFunc)(void*)func;
        }

        class Invoker;
        class Interface;
        class Host;

        const uint32_t MAX_STACK = 64;
        const uint32_t MAX_INVOKERS = 32;
        const uint32_t MAX_INTERFACES = 32;


        //
        // Args structure is the base class for all args
        //

        namespace Flags
        {
            enum Flag
            {
                NonBlocking         = 1 << 0, // do no block waiting for a reply
                ReplyWithArgs       = 1 << 1, // args data should be returned (potentially changed)
                ReplyWithPayload    = 1 << 2, // payload data should be returned (potentially changed)
            };
        }

        struct HELIUM_FOUNDATION_API Args
        {
            Host* m_Host;
            uint32_t   m_Flags;

            void* m_Payload;
            uint32_t   m_PayloadSize;
        };
        typedef Helium::Signature< RPC::Args&>::Delegate ArgsDelegate;


        //
        // Invoker:
        //  - packages an invocation for dispatch to a remote implementation
        //  - performs invocation on locally defined virtual implementation
        //

        class Invoker : public Helium::RefCountBase< Invoker >
        {
        public:
            Invoker (Interface* iface, SwizzleFunc swizzler)
                : m_Interface (iface)
                , m_Swizzler (swizzler)
            {
                HELIUM_ASSERT( iface && swizzler );
            }

            virtual ~Invoker()
            {

            }

            virtual uint32_t GetArgsSize()
            {
                return 0;
            }

            virtual void Invoke(uint8_t* data, uint32_t size) = 0;

            const char* GetName()
            {
                return m_Name;
            }

            Interface* GetInterface()
            {
                return m_Interface;
            }

            void Swizzle( void* data )
            {
                m_Swizzler( data );
            }

        protected:
            const char*   m_Name;
            Interface*    m_Interface;
            SwizzleFunc   m_Swizzler;
        };

        typedef Helium::SmartPtr< Invoker > InvokerPtr;


        //
        // Interface is a named group of invokers
        //

        class HELIUM_FOUNDATION_API Interface
        {
        public:
            Interface(const char* name);

            Host* GetHost()
            {
                return m_Host;
            }
            void SetHost(Host* host)
            {
                m_Host = host;
            }

            const char* GetName()
            {
                return m_Name;
            }

            void AddInvoker(InvokerPtr invoker);
            Invoker* GetInvoker(const char* name);

        protected:
            const char*   m_Name;
            Host*         m_Host;
            InvokerPtr    m_Invokers[MAX_INVOKERS];
            uint32_t      m_InvokerCount;
        };


        //
        // Host is the endpoint for communication and local store of interfaces
        //

        class HELIUM_FOUNDATION_API Host
        {
        public:
            Host();
            ~Host();

            void Reset();

            //
            // Interface managment
            //

            // set/query local implementations
            void AddInterface(RPC::Interface* iface);
            Interface* GetInterface(const char* name);

            //
            // IPC connection settings
            //

            // set the communication connection to use
            void SetConnection(IPC::Connection* con);

            //
            // Invoker dispatching
            //

            // create message to send
            IPC::Message* Create(Invoker* invoker, uint32_t size, int32_t transaction = 0);

            // helper function to send a single data block
            void Emit(Invoker* invoker, Args* args = NULL, uint32_t size = 0, SwizzleFunc swizzler = NULL);

            // process data from the other side
            bool Invoke(IPC::Message* msg);

            // take the message data (call from within an interface function)
            uint8_t* TakeData();

            // Are we connected and ready to do work?
            bool Connected();

            // Call this periodically in your to dispatch (invoke) rpc functions within the calling thread and return
            bool Dispatch();

        private:
            // Call this to process all messages in the calling thread, pass true to sleep until the connection breaks
            bool Process(bool wait);

        private:
            struct Frame
            {
                IPC::Message*     m_Message;
                bool              m_MessageTaken;

                bool              m_Replied;
                uint32_t               m_ReplyID;
                uint8_t*               m_ReplyData;
                uint32_t               m_ReplySize;
                int32_t               m_ReplyTransaction;
            };

            class HELIUM_FOUNDATION_API Stack
            {
            public:
                Stack();
                void Reset();
                int32_t Size();
                Frame* Push();
                Frame* Top();
                void Pop();

            private:
                Frame             m_Frames[MAX_STACK];
                uint32_t               m_Size;
            };

            IPC::Connection*    m_Connection;
            uint32_t                 m_ConnectionCount;
            Stack               m_Stack;
            Interface*          m_Interfaces[MAX_INTERFACES];
            uint32_t                 m_InterfaceCount;
        };

        template<class ArgsType>
        class InvokerTemplate : public Invoker
        {
        public:
            typedef Helium::Signature< ArgsType&> InvokerSignature;
            typedef typename InvokerSignature::Delegate InvokerDelegate;

            InvokerTemplate(Interface* iface, InvokerDelegate delegate)
                : Invoker (iface, GetSwizzleFunc<ArgsType>())
                , m_Delegate (delegate)
            {

            }

            virtual uint32_t GetArgsSize()
            {
                return sizeof(ArgsType);
            }

            virtual void Invoke(uint8_t* data, uint32_t size)
            {
                if (size)
                {
                    m_Swizzler(data);

                    ArgsType* args = (ArgsType*)data;

                    if ( sizeof(ArgsType) > size )
                    {
                        args->m_Payload = data + sizeof(ArgsType);
                        args->m_PayloadSize = size - sizeof(ArgsType);
                    }

                    m_Delegate.Invoke( *args );
                }
            }

            void Emit(ArgsType* args, void* payload = NULL, uint32_t size = 0)
            {
                args->m_Payload = payload;
                args->m_PayloadSize = size;
                m_Interface->GetHost()->Emit(this, args, sizeof(ArgsType), m_Swizzler);
            }

        private:
            InvokerDelegate m_Delegate;
        };

        namespace Test
        {
            struct TestArgs : RPC::Args
            {
                uint8_t m_char;
                uint32_t m_integer;
            };
            typedef Helium::Signature< TestArgs&>::Delegate TestDelegate;

            class TestInterface : public RPC::Interface
            {
            public:
                TestInterface();

                void Test( TestArgs& args );
            };

            void AddInterface(RPC::Host& host);
        };
    }
}