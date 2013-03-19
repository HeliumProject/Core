#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#if PS3_POSIX
#include <netex/net.h>
#include <netex/ifctl.h>
#include <netex/errno.h>
#elif defined(HELIUM_OS_LINUX)
#include <errno.h>
#endif
#include <arpa/inet.h>

namespace Helium
{
    typedef int Socket;
}
