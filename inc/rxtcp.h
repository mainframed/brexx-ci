#ifndef __RXTCP_H
#define __RXTCP_H

#ifdef JCC
#include <io.h>
#include <time.h>
#include "sockets.h"
#include "mvsutils.h"
#include "rxmvsext.h"
#define __unused
#elif WIN32
#else
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <netinet/ip.h> /* superset of previous */
# include <netdb.h>
# include <arpa/inet.h>
# include <errno.h>
# define SOCKET      long
# define SOCKADDR_IN struct sockaddr_in
# define LPSOCKADDR  struct sockaddr *
# define SOCKET      long
# define INVALID_SOCKET (-1)
# define SOCKET_ERROR   (-1)
# define WSAGetLastError() errno
#endif

void RxTcpRegFunctions();

#endif //__RXTCP_H
