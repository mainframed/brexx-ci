#include <fcntl.h>
#include <sys/ioctl.h>
#include "rexx.h"
#include "rxdefs.h"
#include "rxtcp.h"
#include "lstring.h"

#ifndef WIN32   // don't compile in Windows

void printerrno()
{
    int errnum = errno;
    fprintf(stderr, "Value of errno: %d\n", errno);
    perror("Error printed by perror");
    fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));    
}

void R_tcpopen(__unused int func)
{
    int rc = 0;
    int flag;

    unsigned long inAddress;
    unsigned int port;
    unsigned int timeout;

    struct sockaddr_in sockAddrIn;
    struct hostent *host;
    struct timeval timeoutValue;

    fd_set fdsRead, fdsWrite, fdsError;

    SOCKET Ccom_han;

    if (ARGN < 2) Lerror(ERR_INCORRECT_CALL, 0);
    if (ARGN > 3) Lerror(ERR_INCORRECT_CALL, 0);

    LASCIIZ(*ARG1)
    get_s(1)
    get_i(2, port)
    if (ARGN == 3) {
        get_i(3, timeout)
    } else {
        timeout = 5;
    }

    // set receive timeout
    timeoutValue.tv_sec = timeout;
    timeoutValue.tv_usec = 0;

    // get internet address
    inAddress = inet_addr((const char *) LSTR(*ARG1));
    if ((inAddress) == INADDR_NONE) {
        host = gethostbyname(LSTR(*ARG1));
        if (host == NULL || host->h_addr_list[0] == NULL) {
            rc = -5;
        } else {
            inAddress = ((long *) (host->h_addr_list[0]))[0];
        }
    }

    // create socket
    if (rc == 0) {
        sockAddrIn.sin_family = AF_INET;
        sockAddrIn.sin_addr.s_addr = inAddress;
        sockAddrIn.sin_port = ntohs (port);

        Ccom_han = socket(PF_INET, SOCK_STREAM, 0);
        if (Ccom_han == INVALID_SOCKET) {
            rc = -4;
        }
    }

    if (rc == 0) {
#ifdef __CROSS__
        flag = 1;
        ioctl (Ccom_han, FIONBIO, &flag);
#endif
        rc = connect(Ccom_han, (LPSOCKADDR) &sockAddrIn, sizeof(sockAddrIn));
        if (errno == EINPROGRESS) rc = 0;
    }

    if (rc == 0) {
        FD_ZERO(&fdsRead);
        FD_ZERO(&fdsWrite);
        FD_ZERO(&fdsError);

        FD_SET(Ccom_han, &fdsRead);
        FD_SET(Ccom_han, &fdsWrite);
        FD_SET(Ccom_han, &fdsError);

        // check if the socket is ready
        select(Ccom_han + 1, &fdsRead, &fdsWrite, &fdsError, &timeoutValue);
        if (!FD_ISSET(Ccom_han, &fdsRead) && !FD_ISSET(Ccom_han, &fdsWrite))
        {
            rc = -3;
        }

#ifdef __CROSS1__
        flag = 0;
        ioctl (Ccom_han, FIONBIO, &flag);
#endif

    }

    if (rc == 0) {
        // return socket fd
        LINT(*ARGR)  = Ccom_han;
    } else {
        // return internal error code
        LINT(*ARGR) = rc;
    }

    LTYPE(*ARGR) = LINTEGER_TY;
    LLEN(*ARGR)  = sizeof(long);
}

void R_tcpsend(__unused int func)
{
    SOCKET Ccom_han;

    ssize_t rc;
    long j;

    SOCKADDR_IN Clocal_adx;
    struct hostent *result;
    char buffer[1024];

    L2INT(ARG1);
    Ccom_han = LINT(*ARG1);

    strcpy (buffer, (char *)LSTR(*ARG2));
    printf("DBG> sending [%s] to server\n", buffer);

    j = strlen(buffer);
#ifdef JCC
    ebcdic2ascii (buffer, j);
#endif

    rc = send(Ccom_han, buffer, j, 0);
    if (rc == SOCKET_ERROR) {
        printf("ERR> send failed, terminating.\n");
#ifdef JCC
        closesocket (Ccom_han);
#endif
        Lerror(ERR_INCORRECT_CALL, 0);
    }

}

void R_tcprecv(__unused int func)
{
    int sock;

    long j;
    char buffer[1024];

#ifdef JCC
    int                 lastError = 0;
#endif

    L2INT(ARG1);
    sock = LINT(*ARG1);

    // receive data
    if ((j = recv(sock, buffer, sizeof(buffer), 0)) == SOCKET_ERROR) {
        printf("ERR> receiving failed.\n");
        Lerror(ERR_INCORRECT_CALL, 0);
        /*
        if (WSAGetLastError() != ENOTSOCK)
            _putline ("recv failed, terminating.");
        running = 0;
        break;*/
    };
#ifdef JCC
    ascii2ebcdic (buffer, j);
#endif

    if (buffer[0] == 55) {
        printf("DBG> terminating at EOT.\n");
    }

    // print to terminal
    buffer[j] = 0;

    printf("DBG> received [%s] from server\n", buffer);

}

void R_tcpsend2(__unused int func)
{
    SOCKET Ccom_han;

    long j;
    int sockerr = 0;
    char ip_adx[260];
    SOCKADDR_IN Clocal_adx;
    struct hostent *result;
    char buffer[1024];
    char newline[2] = {0x15, 0x00};

    L2INT(ARG1);
    Ccom_han = LINT(*ARG1);

    strcpy (buffer, LSTR(*ARG2));
    printf("DBG> sending [%s] to server\n", buffer);

    j = strlen(buffer);
#ifdef JCC
    ebcdic2ascii (buffer, j);
#endif

    if (send(Ccom_han, buffer, j, 0) == SOCKET_ERROR) {
        printf("ERR> send failed, terminating.\n");
#ifdef JCC
        closesocket (Ccom_han);
#endif
        Lerror(ERR_INCORRECT_CALL, 0);
    }

}

#endif    // not in Windows

/* register rexx functions to brexx/370 */
void RxTcpRegFunctions() {
#ifndef WIN32   // don't compile in Windows
    RxRegFunction("TCPOPEN", R_tcpopen, 0);
    RxRegFunction("TCPRECEIVE", R_tcprecv, 0);
    RxRegFunction("TCPSEND", R_tcpsend, 0);
#endif
} /* RxNetDataRegFunctions() */



