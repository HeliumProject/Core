#include "Precompile.h"
#include "Socket.h"

#include "Platform/Console.h"
#include "Platform/Assert.h"

#if HELIUM_OS_LINUX
# include <pthread.h>
# include <signal.h>
#endif

using namespace Helium;

const static int InvalidHandleValue = -1;

bool Helium::InitializeSockets()
{
#if HELIUM_OS_LINUX
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
	return HELIUM_VERIFY( s == 0 );
#else
	return true;
#endif
}

void Helium::CleanupSockets()
{
}

void Helium::InitializeSocketThread()
{
}

void Helium::CleanupSocketThread()
{
}

int Helium::GetSocketError()
{
	return errno;
}

Socket::Socket()
	: m_Handle( InvalidHandleValue )
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

void Socket::Close()
{
	if ( m_Handle >= 0 )
	{
		::shutdown(m_Handle, SHUT_RDWR);
		HELIUM_VERIFY( ::close(m_Handle) == 0 );
		m_Handle = InvalidHandleValue;
	}
}

bool Socket::Bind(uint16_t port)
{
	int reuse = 1;
	::setsockopt( m_Handle, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse) );

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(port);
	if ( ::bind(m_Handle, (sockaddr*)&service, sizeof(sockaddr_in) ) == 0 )
	{
		return true;
	}
	else
	{
		Helium::Print("Failed to bind socket %d (%d)\n", m_Handle, Helium::GetSocketError());
		::shutdown(m_Handle, SHUT_RDWR);
		HELIUM_VERIFY( ::close(m_Handle) != 0 );
		m_Handle = InvalidHandleValue;
		return false;
	}
}

bool Socket::Listen()
{
	if ( HELIUM_VERIFY( m_Protocol == Helium::SocketProtocols::Tcp ) )
	{
		if ( ::listen(m_Handle, SOMAXCONN) == 0 )
		{
			return true;
		}
		else
		{
			Helium::Print("Failed to listen socket %d (%d)\n", m_Handle, Helium::GetSocketError());
			::shutdown(m_Handle, SHUT_RDWR);
			HELIUM_VERIFY( ::close(m_Handle) != 0 );
			m_Handle = InvalidHandleValue;
			return false;
		}
	}

	return false;
}

bool Socket::Connect( uint16_t port, const char* ip )
{
	if ( HELIUM_VERIFY( m_Protocol == Helium::SocketProtocols::Tcp ) )
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_BROADCAST);
		addr.sin_port = htons(port);
		if ( ::connect(m_Handle, (sockaddr*)&addr, sizeof(addr)) == 0 )
		{
			return true;
		}
	}

	return false;
}

bool Socket::Accept(Socket& server_socket, sockaddr_in* client_info)
{
	if ( HELIUM_VERIFY( m_Protocol == Helium::SocketProtocols::Tcp ) )
	{
		socklen_t lengthname = sizeof(sockaddr_in);
		m_Handle = ::accept( server_socket.m_Handle, (struct sockaddr *)client_info, &lengthname );
		return m_Handle != InvalidHandleValue;
	}

	return false;
}

bool Socket::Read(void* buffer, uint32_t bytes, uint32_t& read, sockaddr_in* peer)
{
	sockaddr_in addr;
	socklen_t addrLen = sizeof( addr );
	bool udp = m_Protocol == SocketProtocols::Udp;
	uint32_t flags = MSG_WAITALL;

	int32_t local_read;
	if ( udp )
	{
		local_read = ::recvfrom( m_Handle, (char*)buffer, bytes, flags, (sockaddr*)(peer ? peer : &addr), &addrLen );
	}
	else
	{
		local_read = ::recv( m_Handle, (char*)buffer, bytes, flags );
	}

	if (local_read < 0)
	{
		return false;
	}

	read = local_read;

	return true;
}

bool Socket::Write(void* buffer, uint32_t bytes, uint32_t& wrote, const char* ip, uint16_t port)
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
		{
			return false;
		}
	}

	uint32_t addrLen = sizeof( addr );
	int32_t local_wrote;
	if ( udp )
	{
		local_wrote = ::sendto( m_Handle, (char*)buffer, bytes, 0, (sockaddr*)&addr, addrLen );
	}
	else
	{
		local_wrote = ::send( m_Handle, (char*)buffer, bytes, 0 );
	}

	if (local_wrote < 0)
	{
		return false;
	}

	wrote = local_wrote;

	return true;
}

int Socket::Select(Handle range, fd_set* read_set, fd_set* write_set, timeval* timeout)
{
	return ::select(range, read_set, write_set, 0, timeout);
}
