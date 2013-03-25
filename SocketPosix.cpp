#include "Platform/Socket.h"

#include "Platform/Console.h"
#include "Platform/Assert.h"

using namespace Helium;

#ifdef PS3_POSIX
# define close close
# define select select
# define errno sys_net_errno
#endif

bool Helium::InitializeSockets()
{
    return true;
}

void Helium::CleanupSockets()
{

}

void Helium::CleanupSocketThread()
{
#ifdef PS3_POSIX
    // Cleaning up just a single thread doesn't seem to actually work
    i32 ret = sys_net_free_thread_context( 0, SYS_NET_THREAD_ALL );

    if ( ret < 0 )
    {
        Helium::Print( "TCP Support: Failed to cleanup thread context (%d)\n", Helium::GetSocketError() );
    }
#endif
}

int Helium::GetSocketError()
{
    return errno;
}

bool Helium::CreateSocket(Socket& socket, SocketProtocol protocol)
{
    if (protocol == SocketProtocols::Tcp)
    {
        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
    }

    if (socket < 0)
    {
        Helium::Print("TCP Support: Failed to create socket %d (%d)\n", socket, Helium::GetSocketError());
        return false;
    }

    return true;
}

bool Helium::CloseSocket(Socket& socket)
{
    // don't bother to check for errors here, as this socket may not have been communicated through yet
    ::shutdown(socket, SHUT_RDWR);

    if ( ::close(socket) < 0 )
    {
        Helium::Print("TCP Support: Failed to close socket %d (%d)\n", socket, Helium::GetSocketError());
        return false;
    }

    return true;
}

bool Helium::BindSocket(Socket& socket, uint16_t port)
{
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);
    if ( ::bind(socket, (sockaddr*)&service, sizeof(sockaddr_in) ) < 0 )
    {
        Helium::Print("TCP Support: Failed to bind socket %d (%d)\n", socket, Helium::GetSocketError());

        if (shutdown(socket, SHUT_RDWR) < 0)
        {
            HELIUM_BREAK();
        }
        if (close(socket) < 0) {
            HELIUM_BREAK();
        }

        return false;
    }

    return true;
}

bool Helium::ListenSocket(Socket& socket)
{
    if ( ::listen(socket, 5) < 0 )
    {
        Helium::Print("TCP Support: Failed to listen socket %d (%d)\n", socket, Helium::GetSocketError());

        if (shutdown(socket, SHUT_RDWR) < 0)
        {
            HELIUM_BREAK();
        }
        if (close(socket) < 0)
        {
            HELIUM_BREAK();
        }

        return false;
    }

    return true;
}

bool Helium::ConnectSocket(Socket& socket, sockaddr_in* service)
{
    return ::connect(socket, (struct sockaddr *)service, sizeof(sockaddr_in)) >= 0;
}

bool Helium::AcceptSocket(Socket& socket, Socket& server_socket, sockaddr_in* client_info)
{
    socklen_t lengthname = sizeof(sockaddr_in);

    socket = ::accept( server_socket, (struct sockaddr *)client_info, &lengthname );

    return socket > 0;
}

int Helium::SelectSocket(int range, fd_set* read_set, fd_set* write_set, struct timeval* timeout)
{
    return ::select(range, read_set, write_set, 0, timeout);
}

bool Helium::ReadSocket(Socket& socket, void* buffer, uint32_t bytes, uint32_t& read, Condition& terminate, sockaddr_in *_peer)
{
    int proto = 0;
    socklen_t optlen = sizeof(int);
    HELIUM_ASSERT( ::getsockopt(socket, SOL_SOCKET, SO_PROTOCOL, &proto, &optlen) );
    HELIUM_ASSERT( proto != IPPROTO_UDP );

    int32_t local_read = ::recv( socket, (tchar_t*)buffer, bytes, 0 );

    if (local_read < 0)
    {
        return false;
    }

    read = local_read;

    return true;
}

bool Helium::WriteSocket(Socket& socket, void* buffer, uint32_t bytes, uint32_t& wrote, Condition& terminate, sockaddr_in *_peer)
{
    int proto = 0;
    socklen_t optlen = sizeof(int);
    HELIUM_ASSERT( ::getsockopt(socket, SOL_SOCKET, SO_PROTOCOL, &proto, &optlen) );
    HELIUM_ASSERT( proto != IPPROTO_UDP );

    int32_t local_wrote = ::send( socket, (tchar_t*)buffer, bytes, 0 );

    if (local_wrote < 0)
    {
        return false;
    }

    wrote = local_wrote;

    return true;
}
