#pragma once

#include "Platform/Condition.h"
#include "Platform/Types.h"
#include "Utility.h"

#if HELIUM_OS_WIN
typedef int socklen_t;
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/poll.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <errno.h>
# include <unistd.h>
#endif

namespace Helium
{
    // Call to initiate/shutdown the subsystem, reference counted
    HELIUM_PLATFORM_API bool InitializeSockets();
    HELIUM_PLATFORM_API void CleanupSockets();

    // Call to initiate/shutdown a thread that will call into the api
    HELIUM_PLATFORM_API void InitializeSocketThread();
    HELIUM_PLATFORM_API void CleanupSocketThread();

    // Get the most recent socket error
    HELIUM_PLATFORM_API int GetSocketError();

    namespace SocketProtocols
    {
        enum SocketProtocol
        {
            Tcp,
            Udp
        };
    }
    typedef SocketProtocols::SocketProtocol SocketProtocol;

    class HELIUM_PLATFORM_API Socket : NonCopyable
    {
    public:
#if HELIUM_OS_WIN
        typedef SOCKET Handle;
#else
        typedef int Handle;
#endif
        Socket();
        ~Socket();

        operator Handle()
        {
            return m_Handle;
        }

        // Create/close sockets
        bool Create(SocketProtocol protocol);
        void Close();

        // Associate the socket with a particular port
        bool Bind(uint16_t port);

        // These functions are invalid for connectionless protocols such as UDP
        bool Listen();
        bool Connect(uint16_t port, const char* ip = NULL);
        bool Accept(Socket& server_socket, sockaddr_in* client_info);

        // Use the socket to communicate on a connection based protocol
        bool Read(void* buffer, uint32_t bytes, uint32_t& read, sockaddr_in* peer = NULL);
        bool Write(void* buffer, uint32_t bytes, uint32_t& wrote, const char *ip = NULL, uint16_t port = 0);

        // Poll the state of a socket
        static int Select( Handle range, fd_set* read_set, fd_set* write_set, struct timeval* timeout);

    private:
        Handle         m_Handle;
        SocketProtocol m_Protocol;

#if HELIUM_OS_WIN
        OVERLAPPED     m_Overlapped;
        HANDLE         m_TerminateIo;
#endif
    };   
}
