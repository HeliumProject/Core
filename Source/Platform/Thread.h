#pragma once

#include "Platform/Utility.h"

#ifndef WIN32
# include <pthread.h>
#endif

namespace Helium
{
	namespace ThreadPriorities
	{
		/// Thread priority constants.
		enum Type
		{
			/// Lowest thread priority.
			Lowest,
			/// Low thread priority.
			Low,
			/// Normal (default) thread priority.
			Normal,
			/// High thread priority.
			High,
			/// Highest thread priority.
			Highest,
			/// Do not set a priority (inherit).
			Inherit,
		};
	}
	typedef ThreadPriorities::Type ThreadPriority;

#if HELIUM_OS_WIN
	/// Thread ID type.
	typedef uint32_t       ThreadId;
	/// Invalid ThreadId
	const extern uint32_t  InvalidThreadId;
#elif HELIUM_OS_LINUX
	/// Posix threads are identified as process IDs
	typedef pid_t          ThreadId;
	/// Invalid ThreadId
	const extern pid_t     InvalidThreadId;
#elif HELIUM_OS_MAC
	/// Posix threads are identified as process IDs
	typedef pthread_t      ThreadId;
	/// Invalid ThreadId
	const extern pthread_t InvalidThreadId;
#endif

	/// Thread interface.
	class HELIUM_PLATFORM_API Thread : NonCopyable
	{
	private:
#if HELIUM_OS_WIN
		/// Platform-specific thread handle type.
		typedef void*     Handle;
#else

		/// Posix threads handle
		typedef pthread_t Handle;
#endif

		/// Platform-specific thread handle.
		Handle m_Handle;

#if !HELIUM_OS_WIN
		/// Validity of the handle value
		bool m_Valid;
#endif
		/// Thread name.
		char m_Name[ 128 ];

		/// @name Thread Callback
		//@{
#if HELIUM_OS_WIN
		static unsigned int __stdcall ThreadCallback( void* pData );
#else
		static void* ThreadCallback( void* pData );
#endif
		//@}

	public:
		/// @name Construction/Destruction
		//@{
		explicit Thread();
		virtual ~Thread();
		//@}

		/// @name Data Access
		//@{
		inline const char* GetName() const;
		//@}

		/// @name Caller Interface
		//@{
		bool Start( const char* pName, ThreadPriority priority = ThreadPriorities::Inherit );
		bool Join( uint32_t timeOutMilliseconds = 0 );
		bool TryJoin();
		bool IsValid() const;
		//@}

		/// @name Thread-side Interface
		//@{
		virtual void Run() = 0;
		//@}

		/// @name Static Functions
		//@{
		static void Sleep( uint32_t milliseconds );

		/// Yield the remainder of the calling thread's time slice for other threads of equal priority.
		///
		/// If other threads of equal priority are awaiting execution, this will immediately yield execution to those
		/// threads.  If no other threads of equal priority are waiting, the thread will continue execution immediately.
		///
		/// To yield control to lower-priority threads, Sleep() should be called instead with a non-zero amount of time.
		///
		/// @see Sleep()
		static void Yield();

		/// Get the ID of the thread in which this function is called.
		///
		/// @return  Current thread ID.
		static ThreadId GetCurrentId();

		/// Get the ID of the thread in which main was called.
		///
		/// @return  Main thread ID.
		static ThreadId GetMainId();

		/// Check if the calling thread is the thread that called main().
		///
		/// @return  Returns true if the calling thread is the same thread that called main().
		static bool IsMain();
		//@}
	};

	/// Interface for thread execution.
	///
	/// This class should be implemented to provide the code to be executed by a thread.  When a Thread instance starts,
	/// it will call the Run() method within the context of the running thread.  The Thread instance can be checked to
	/// determine whether or not a thread is still running.
	class HELIUM_PLATFORM_API Runnable
	{
	public:
		/// @name Construction/Destruction
		//@{
		virtual ~Runnable() = 0;
		//@}

		/// @name Runnable Interface
		//@{
		virtual void Run() = 0;
		//@}
	};

	/// Thread interface for creating threads that execute a Runnable object.
	class HELIUM_PLATFORM_API RunnableThread : public Thread
	{
	public:
		/// @name Construction/Destruction
		//@{
		//explicit Thread( Runnable* pRunnable = NULL, const String& rName = String() );
		explicit RunnableThread( Runnable* pRunnable = NULL );
		virtual ~RunnableThread();
		//@}

		/// @name Caller Interface
		//@{
		void SetRunnable( Runnable* pRunnable );
		//@}

		/// @name Thread-side Interface
		//@{
		virtual void Run();
		//@}

	private:
		/// Runnable to execute.
		Runnable* m_pRunnable;
	};

	/// Thread interface for easily creating threads based on a C function or C++ member function pointer.
	class HELIUM_PLATFORM_API CallbackThread : public Thread
	{
	public:
		/// Thread callback type.
		typedef void ( *Entry )( void* );

	private:
		struct ThreadHelperArgs
		{
			ThreadHelperArgs( void* object, void* args )
				: m_Object (object)
				, m_Args (args)
			{
			}

			void* m_Object;
			void* m_Args;
		};

		/// Thread callback.
		Entry m_Entry;
		/// Thread callback parameter.
		void* m_Object;

	public:
		/// @name Construction/Destruction
		//@{
		CallbackThread();
		virtual ~CallbackThread();
		//@}

		/// @name Caller Interface
		//@{
		// create and execute a thread
		bool Create( Entry entry, void* obj, const char* name, ThreadPriority priority = ThreadPriorities::Inherit );

		// C++ helper (remember, it is valid to pass a member function pointer as a template parameter!)
		template< class ObjectT, void (ObjectT::*method)() >
		static void EntryHelper( void* param );

		// create and execute a thread with a separate args object
		inline bool CreateWithArgs( Entry entry, void* obj, void* args, const char* name, ThreadPriority priority = ThreadPriorities::Inherit );

		// C++ helper (remember, it is valid to pass a member function pointer as a template parameter!)
		template< class ObjectT, typename ArgsT, void (ObjectT::*method)( ArgsT& ) >
		static void EntryHelperWithArgs( void* param );
		//@}

		/// @name Thread-side Interface
		//@{
		virtual void Run();
		//@}
	};

	class HELIUM_PLATFORM_API ThreadLocalPointer
	{
	public:
		ThreadLocalPointer();
		~ThreadLocalPointer();

		void* GetPointer() const;
		void SetPointer(void* value);

	protected:
#if HELIUM_OS_WIN
		unsigned long m_Key;
#else
		pthread_key_t m_Key;
#endif
	};

	template< class T >
	class ThreadLocal : public ThreadLocalPointer
	{
	public:
		T* GetPointer() const;
		void SetPointer(T* value);
	};
}

#include "Platform/Thread.inl"
