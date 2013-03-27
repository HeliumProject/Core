#pragma once

#include "Platform/Types.h"
#include "Platform/Condition.h"

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

    class HELIUM_PLATFORM_API Socket
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
        bool Close();

        // Associate the socket with a particular port
        bool Bind(uint16_t port);

        // These functions are invalid for connectionless protocols such as UDP
        bool Listen();
        bool Connect(const tchar_t* ip, uint16_t port);
        bool Accept(Socket& server_socket, sockaddr_in* client_info);

        // Use the socket to communicate on a connection based protocol
        bool Read(void* buffer, uint32_t bytes, uint32_t& read, Condition& terminate, sockaddr_in* peer = NULL);
        bool Write(void* buffer, uint32_t bytes, uint32_t& wrote, Condition& terminate, const tchar_t *ip = NULL, uint16_t port = 0);

        // Poll the state of a socket
        static int Select( Handle range, fd_set* read_set, fd_set* write_set, struct timeval* timeout);

    private:
        Handle     m_Handle;

#if HELIUM_OS_WIN
        int        m_Protocol;
        OVERLAPPED m_Overlapped;
#endif
    };   
}
