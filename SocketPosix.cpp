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

void Helium::InitializeSocketThread()
{

}

void Helium::CleanupSocketThread()
{
#ifdef PS3_POSIX
    // Cleaning up just a single thread doesn't seem to actually work
    if ( sys_net_free_thread_context( 0, SYS_NET_THREAD_ALL ) != 0 )
    {
        Helium::Print( "Failed to cleanup thread context (%d)\n", Helium::GetSocketError() );
    }
#endif
}

int Helium::GetSocketError()
{
    return errno;
}

Socket::Socket()
: m_Handle( -1 )
, m_Protocol( SocketProtocols::Tcp )
{

}

Socket::~Socket()
{
    Close();
}

bool Socket::Create(SocketProtocol protocol)
{
    bool result = false;

    m_Protocol = protocol;
    if (m_Protocol == SocketProtocols::Tcp)
    {
        m_Handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        m_Handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    if ( m_Handle >= 0 )
    {
        result = true;
    }
    else
    {
        Helium::Print("Failed to create socket %d (%d)\n", m_Handle, Helium::GetSocketError());
    }

    return result;
}

bool Socket::Close()
{
    bool result = false;

    if ( m_Handle >= 0 )
    {
        // don't bother to check for errors here, as this socket may not have been communicated through yet
        ::shutdown(m_Handle, SHUT_RDWR);

        if ( ::close(m_Handle) == 0 )
        {
            result = true;
            m_Handle = -1;
        }
        else
        {
            Helium::Print("Failed to close socket %d (%d)\n", m_Handle, Helium::GetSocketError());
        }
    }

    return result;
}

bool Socket::Bind(uint16_t port)
{
    bool result = false;

    bool reuse = true;
    ::setsockopt( m_Handle, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse) );

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);
    if ( ::bind(m_Handle, (sockaddr*)&service, sizeof(sockaddr_in) ) == 0 )
    {
        result = true;
    }
    else
    {
        Helium::Print("Failed to bind socket %d (%d)\n", m_Handle, Helium::GetSocketError());

        if ( ::shutdown(m_Handle, SHUT_RDWR) != 0 )
        {
            HELIUM_BREAK();
        }
        if ( ::close(m_Handle) != 0 )
        {
            HELIUM_BREAK();
        }

        result = false;
    }

    return result;
}

bool Socket::Listen()
{
    bool result = false;

    if ( ::listen(m_Handle, 5) == 0 )
    {
        result = true;
    }
    else
    {
        Helium::Print("Failed to listen socket %d (%d)\n", m_Handle, Helium::GetSocketError());

        if ( ::shutdown(m_Handle, SHUT_RDWR) != 0 )
        {
            HELIUM_BREAK();
        }
        if ( ::close(m_Handle) != 0 )
        {
            HELIUM_BREAK();
        }

        return false;
    }

    return true;
}

bool Socket::Connect( uint16_t port, const tchar_t* ip )
{
    bool result = false;

    HELIUM_ASSERT( m_Protocol == Helium::SocketProtocols::Tcp );

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_BROADCAST);
    addr.sin_port = htons(port);
    if ( ::connect(m_Handle, (sockaddr*)&addr, sizeof(addr)) == 0 )
    {
        result = true;
    }

    return result;
}

bool Socket::Accept(Socket& server_socket, sockaddr_in* client_info)
{
    socklen_t lengthname = sizeof(sockaddr_in);
    m_Handle = ::accept( server_socket.m_Handle, (struct sockaddr *)client_info, &lengthname );
    return m_Handle > 0;
}

bool Socket::Read(void* buffer, uint32_t bytes, uint32_t& read, sockaddr_in* peer)
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof( addr );
    bool udp = m_Protocol == SocketProtocols::Udp;
    int32_t local_read = udp ? ::recvfrom( m_Handle, (tchar_t*)buffer, bytes, 0, (sockaddr*)(peer ? peer : &addr), &addrLen ) :
                               ::recv    ( m_Handle, (tchar_t*)buffer, bytes, 0 );
    if (local_read < 0)
    {
        return false;
    }

    read = local_read;

    return true;
}

bool Socket::Write(void* buffer, uint32_t bytes, uint32_t& wrote, const tchar_t* ip, uint16_t port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_BROADCAST);

    bool udp = m_Protocol == SocketProtocols::Udp;
    if (udp)
    {
	int opt = ip ? 0 : 1;
        int err = ::setsockopt( m_Handle, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt) );
        if (err < 0)
          return false;
    }

    uint32_t addrLen = sizeof( addr );
    int32_t local_wrote = udp ? ::sendto( m_Handle, (tchar_t*)buffer, bytes, 0, (sockaddr*)&addr, addrLen ) :
                                ::send  ( m_Handle, (tchar_t*)buffer, bytes, 0 );
    if (local_wrote < 0)
    {
        return false;
    }

    wrote = local_wrote;

    return true;
}

int Socket::Select(Handle range, fd_set* read_set, fd_set* write_set, struct timeval* timeout)
{
    return ::select(range, read_set, write_set, 0, timeout);
}
