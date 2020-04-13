/*
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

/**
 * @file
 *
 * Implementation of pcsl_network.h for platforms that support the winsock 
 * API.
 *
 * For all functions, the "handle" is the winsock handle (an int)
 * cast to void *.  Since winsock reads and writes to sockets are synchronous,
 * the context for reading and writing is always set to NULL.
 */

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#include "javacall_network.h"
#include "javacall_socket.h"
#include "javacall_datagram.h"
#include "javacall_logging.h"

#define WM_DEBUGGER      (WM_USER)
#define WM_HOST_RESOLVED (WM_USER + 1)
#define WM_NETWORK       (WM_USER + 2)

static HWND hPhantomWindow = NULL;
HWND getPhantomWindowHandle();

#define MAX_HOST_LENGTH 256
#define ENABLE_NETWORK_TRACING
static char print_buffer[64];


typedef struct _AddrContext {
    struct _AddrContext* next;
    HANDLE asyncHandle;
    /* host must be last because it is variable length. */
    struct hostent host; 
} AddrContext;

/* For use by pcsl_network_error. */
static int lastError;

/*
 * To reduce complexity, a whole struct is used a root instead of a just a
 * pointer. The root host field never has any real data.
 */
static AddrContext rootAddrContext;

static void addAddrContext(AddrContext* pContext) {
    pContext->next = rootAddrContext.next;
    rootAddrContext.next = pContext;
}

static AddrContext* removeAddrContext(HANDLE asyncHandle) {
    AddrContext* prev = &rootAddrContext;
    AddrContext* current = rootAddrContext.next;

    // Start after the root, because the root struct has no data.
    while (NULL != current) {
        if (current->asyncHandle == asyncHandle) {
            prev->next = current->next;
            return current;
        }

        prev = current;
        current = current->next;
    }

    return NULL;
}

javacall_result javacall_socket_open(javacall_ip_version ip_version,
                                     javacall_handle* pHandle) {
    SOCKET s;
	int falsebuf = 0;
	int status;
    unsigned long nonBlockingFlag = 1;
    
	s = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == s) {

        return JAVACALL_FAIL;
    }

    status = setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                        (char*)&falsebuf, sizeof (falsebuf));
    lastError = WSAGetLastError();
    if (SOCKET_ERROR == status) {
        closesocket(s);
		return JAVACALL_FAIL;
    }
	
	//make socket non-blocking
     status = ioctlsocket(s, FIONBIO, &nonBlockingFlag);
	 lastError = WSAGetLastError();
     if(status ==SOCKET_ERROR) {
         closesocket(s);
         return JAVACALL_FAIL;
     }

	 *pHandle = (javacall_handle)s;
	 return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_connect_start(
	javacall_handle handle,
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
    unsigned char *ipBytes, 
	int port, 
	void **pContext)
{
	SOCKET s = (SOCKET)handle;
    int status;
    struct sockaddr_in addr;
	
    if (ipBytes == NULL)
        return JAVACALL_INVALID_ARGUMENT;

	if (ip_version != JAVACALL_IP_VERSION_4)
		return JAVACALL_INVALID_ARGUMENT;

	//struct hostent *phostent = (struct hostent*)gethostbyname(ipBytes);
	//if (phostent == NULL) {
    //  return JAVACALL_FAIL;
    //}

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);

    //memcpy(&addr.sin_addr, phostent->h_addr, phostent->h_length);
    memcpy((char*)&addr.sin_addr, (char*)ipBytes, 4);
	if(WSAAsyncSelect(s, getPhantomWindowHandle(), WM_NETWORK, FD_CONNECT) != 0) {
      	closesocket(s);
    	return JAVACALL_FAIL;
    }
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    lastError = WSAGetLastError();
    
    *pContext = NULL;

    if (status == 0) {
     /*
      * connect() returned 0, this because the server is on the same host
      * and the loopback address was used to connect. Just return the
      * the handle.
      */

		
        return JAVACALL_OK;
    } /* end of connect() is OK */

    if (WSAEWOULDBLOCK == lastError) {
        return JAVACALL_WOULD_BLOCK;
    }

    closesocket(s);          	

   return JAVACALL_CONNECTION_NOT_FOUND; 
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_socket_connect_finish(void *handle,void *context){
    SOCKET s = (SOCKET)handle;
    int status = (int)context;

    /* Reset Window's message notification for this connection. */
    WSAAsyncSelect(s, getPhantomWindowHandle(), 0, 0);

    lastError = status;
    if (0 == status) {
        return JAVACALL_OK;
    }

    closesocket(s);
    return JAVACALL_FAIL;
}
      

/**
 * Common implementation between read_start() and read_finish().
 */
static int winsock_read_common(void *handle,
                               unsigned char *pData,
                               int len,
                               int *pBytesRead){

    SOCKET s = (SOCKET)handle;
    int bytesRead;

    bytesRead = recv(s, (char*)pData, len, 0);
	lastError = WSAGetLastError();

    if (SOCKET_ERROR != bytesRead) {
        *pBytesRead = bytesRead;
        return JAVACALL_OK;
    }

    if (WSAEWOULDBLOCK == lastError) {
        /*
         * Win32 only has one notifier per socket so always set both and close,
         * the MIDP event code can handle any extra notifications.
         * Do not cancel the notifications until the socket is closed.
         */
        WSAAsyncSelect(s, getPhantomWindowHandle(), WM_NETWORK,
                       FD_READ | FD_WRITE | FD_CLOSE);
		return JAVACALL_WOULD_BLOCK;
    }

    if (WSAEINTR == lastError ||  WSAENOTSOCK == lastError) {
		return JAVACALL_INTERRUPTED;
    }
	
    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_start(void *handle,
                                           unsigned char *pData,
                                           int len,  
                                           int *pBytesRead,
                                           void **pContext){

    *pContext = NULL;
    return winsock_read_common(handle, pData, len, pBytesRead);
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_finish(void *handle,
                                            unsigned char *pData,
                                            int len,
                                            int *pBytesRead,
                                            void *context){

    (void)context;
    return winsock_read_common(handle, pData, len, pBytesRead);
}


/**
 * Common implementation between write_start() and write_finish().
 */
static int winsock_write_common(void *handle,
                                char *pData,
                                int len,
                                int *pBytesWritten){

    SOCKET s = (SOCKET)handle;
    int bytesSent;

    bytesSent = send(s, pData, len, 0);
    lastError = WSAGetLastError();

    if (SOCKET_ERROR != bytesSent) {
        *pBytesWritten = bytesSent;
        return JAVACALL_OK;
    }

    if (WSAEWOULDBLOCK == lastError) {
        /*
         * Win32 only has one notifier per socket so always set both and close,
         * the MIDP event code can handle any extra notifications.
         * Do not cancel the notifications until the socket is closed.
         */
        WSAAsyncSelect(s, getPhantomWindowHandle(), WM_NETWORK,
                       FD_READ | FD_WRITE | FD_CLOSE);
        return JAVACALL_WOULD_BLOCK;
    }

    if (WSAEINTR == lastError ||  WSAENOTSOCK == lastError) {
        return JAVACALL_INTERRUPTED;
    }

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_start(void *handle,
                                            char *pData,
                                            int len,
                                            int *pBytesWritten,
                                            void **pContext){

    *pContext = NULL;
    return winsock_write_common(handle, pData, len, pBytesWritten);
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_finish(void *handle,
                                             char *pData,
                                             int len,
                                             int *pBytesWritten,
                                             void *context){

    (void)context;
    return winsock_write_common(handle, pData, len, pBytesWritten);
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_available(void *handle, int *pBytesAvailable){
    SOCKET s = (SOCKET)handle;
    unsigned long len = 0;
    int status;

    status = ioctlsocket(s, FIONREAD, &len);
    lastError = WSAGetLastError();
    if (SOCKET_ERROR != status) {
        *pBytesAvailable = (int)len;
        return JAVACALL_OK;
    }

    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_shutdown_output(void *handle) {
    SOCKET s = (SOCKET)handle;
    int status;

    status = shutdown(s, SD_SEND);
    lastError = WSAGetLastError();
    if (SOCKET_ERROR != status) {
        return JAVACALL_OK;
    }

    if (WSAENOTSOCK == lastError) {
        return JAVACALL_INTERRUPTED;
    }

    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 *
 * Note that this function NEVER returns PCSL_NET_WOULDBLOCK. Therefore, the 
 * finish() function should never be called and does nothing.
 */
javacall_result javacall_socket_close_start(void *handle, javacall_bool abort,
                                            void **pContext){

    SOCKET s = (SOCKET)handle;
    int status;

    (void)pContext;

    WSAAsyncSelect(s, getPhantomWindowHandle(), 0, 0);
    /*
     * Unblock any waiting threads, by send a close event with an interrupt
     * status. Closesocket cancels async notitifications on the socket and
     * does NOT send any messages.
     */
    PostMessage(getPhantomWindowHandle(), WM_NETWORK, s,
                WSAMAKESELECTREPLY(FD_CLOSE, WSAEINTR));

    status = closesocket(s);
    lastError = WSAGetLastError();
    if (SOCKET_ERROR != status) {
        return JAVACALL_OK;
    }        

    if (lastError == WSAEWOULDBLOCK) {
        /*
         * Call closesocket again, this will cause the socket to close
         * in the background in a system thread, see doc for closesocket.
         */
        closesocket(s);
        lastError = 0;
        return JAVACALL_OK;
    }

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_close_finish(void *handle,
                                             void *context){

    (void)handle;
    (void)context;
    return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_gethostbyname_start(char *hostname,
                                                     javacall_ip_version ip_version,
                                                     javacall_configuration_id id,
                                                     unsigned char *pAddress, 
                                                     int maxLen,
                                                     int *pLen,
                                                     void **pHandle,
                                                     void **pContext){

    AddrContext* pAddrContext;
    HANDLE asyncHandle;

    (void)pAddress;
    (void)pLen;
    (void)maxLen;
	

    pAddrContext = (AddrContext*)
        malloc((sizeof (AddrContext) + MAXGETHOSTSTRUCT));
    if (NULL == pAddrContext) {
        return JAVACALL_FAIL;
    }

    asyncHandle = WSAAsyncGetHostByName(getPhantomWindowHandle(),
										WM_HOST_RESOLVED, 
										hostname, 
										(char*)(&pAddrContext->host),
                                        MAXGETHOSTSTRUCT);
    lastError = WSAGetLastError();

    if (0 == asyncHandle) {
        free(pAddrContext);
		return JAVACALL_FAIL;
    }

    *pHandle = (void*)asyncHandle;
    pAddrContext->asyncHandle = asyncHandle;
    addAddrContext(pAddrContext);

	return JAVACALL_WOULD_BLOCK;
}


/**
 * See pcsl_network.h for definition.
 *
 * Since the start function never returns PCSL_NET_WOULDBLOCK, this
 * function should never be called.
 */
javacall_result javacall_network_gethostbyname_finish(
                                                      javacall_ip_version ip_version,
                                                      javacall_configuration_id id,
                                                      unsigned char *pAddress,
                                                      int maxLen,
                                                      int *pLen,
                                                      void *handle,
                                                      void *context){

    AddrContext* pAddrContext = removeAddrContext((HANDLE)handle);
    int addrLen;

    lastError = (int)context;
    if (NULL == pAddrContext) {
        return JAVACALL_FAIL;
    }

    if ((0 != lastError) ||
        (AF_INET != pAddrContext->host.h_addrtype &&
         AF_INET6 != pAddrContext->host.h_addrtype)) {
        free(pAddrContext);
        return JAVACALL_FAIL;
    }

    addrLen = pAddrContext->host.h_length;
    if (addrLen > maxLen) {
        free(pAddrContext);
        return JAVACALL_INVALID_ARGUMENT;
    }

    *pLen = addrLen;
	
    memcpy(pAddress, pAddrContext->host.h_addr, addrLen);
    free(pAddrContext);

	return JAVACALL_OK;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_getsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int *pOptval){

    SOCKET s = (SOCKET)handle;
    int level = SOL_SOCKET;
    int optname;
    int optsize = sizeof(optname);
    struct linger lbuf ;
    void * opttarget = (void *) pOptval ;

    switch (flag) {
    case JAVACALL_SOCK_DELAY: /* DELAY */
        level = IPPROTO_TCP;
        optname = TCP_NODELAY;
        break;

    case JAVACALL_SOCK_LINGER: /* LINGER */
    case JAVACALL_SOCK_ABORT: 
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;
        break;

    case JAVACALL_SOCK_KEEPALIVE: /* KEEPALIVE */
        optname = SO_KEEPALIVE;
        break;

    case JAVACALL_SOCK_RCVBUF: /* RCVBUF */
        optname = SO_RCVBUF;
        break;

    case JAVACALL_SOCK_SNDBUF: /* SNDBUF */
        optname = SO_SNDBUF;
        break;

    default: /* Illegal Argument */
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (0 == getsockopt(s, level,  optname, opttarget, &optsize)) {
        if (SO_LINGER == optname) {
            if(JAVACALL_SOCK_LINGER == flag) {
            /* If linger is on return the number of seconds. */
            *pOptval = (lbuf.l_onoff == 0 ? 0 : lbuf.l_linger) ;
            } else { /* if (JAVACALL_SOCK_ABORT == flag) */
                *pOptval = ( ((lbuf.l_onoff == 1) && (lbuf.l_linger == 0)) ? 1 : 0 );
            }
        }

        lastError = 0;

		return JAVACALL_OK;
    }

    lastError = WSAGetLastError();
    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_setsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int optval){

    SOCKET s = (SOCKET)handle;
    int    level = SOL_SOCKET;
    int    optsize =  sizeof(optval);
    int    optname;
    struct linger lbuf ;
    void * opttarget = (void *) & optval ;

    switch (flag) {
    case JAVACALL_SOCK_DELAY: /* DELAY */
        level = IPPROTO_TCP;
        optname = TCP_NODELAY;
        break;

    case JAVACALL_SOCK_LINGER: /* LINGER */
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;
        if (optval == 0) {
            lbuf.l_onoff = 0;
            lbuf.l_linger = 0;
        } else {
            lbuf.l_onoff = 1;
            lbuf.l_linger = (unsigned short)optval;
        }
        break;

    case JAVACALL_SOCK_KEEPALIVE: /* KEEPALIVE */
        optname = SO_KEEPALIVE;
        break;

    case JAVACALL_SOCK_RCVBUF: /* RCVBUF */
        optname = SO_RCVBUF;
        break;

    case JAVACALL_SOCK_SNDBUF: /* SNDBUF */
        optname = SO_SNDBUF;
        break;

    case JAVACALL_SOCK_ABORT: 
        opttarget = (void *) &lbuf ;
        optsize = sizeof (struct linger);
        optname = SO_LINGER;

        lbuf.l_onoff = 1;
        lbuf.l_linger = (unsigned short)0;
        break;

    default: /* Illegal Argument */
        return JAVACALL_INVALID_ARGUMENT;
    }

    if (0 == setsockopt(s, level,  optname, opttarget, optsize)){
        lastError = 0;
        return JAVACALL_OK;
    }

    lastError = WSAGetLastError();
    return JAVACALL_FAIL;
}

/**
 * Gets the string representation of the local device's IP address.
 * This function returns dotted quad IP address as a string in the 
 * output parameter and not the host name.
 *
 * @param pLocalIPAddress base of char array to receive the local
 *        device's IP address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */

javacall_result 
javacall_network_get_local_ip_address_as_string(javacall_ip_version ip_version,
                                                                    /*OUT*/ char *pLocalIPAddress){

    static char hostname[MAX_HOST_LENGTH];
    struct hostent *pHost;
    struct in_addr* pAddr;

    if (SOCKET_ERROR == gethostname(hostname, sizeof (hostname))) {
        lastError = WSAGetLastError();
        return JAVACALL_FAIL;
    }

    pHost = gethostbyname(hostname);
    lastError = WSAGetLastError();

    if (NULL == pHost) {
        return JAVACALL_FAIL;
    }

    pAddr = (struct in_addr*)(pHost->h_addr);
    strcpy(pLocalIPAddress, inet_ntoa(*pAddr));

	return JAVACALL_OK;
}

/*
 * Returns IP address bytes array.
 * Similar with gethostbyname, but non-blocking one.
 * Example: converts "127.0.0.1" to pointer to 0x0100007f
 */
char* getIPBytes_nonblock(char *hostname) {
    struct hostent *pHost;
    int pAddr;

    pHost = gethostbyname(hostname);

    if (NULL == pHost) {
        return 0;
    }

    pAddr = *((int*)pHost->h_addr);

	return pHost->h_addr;
}

extern javacall_result init_wma_emulator();
extern javacall_result finalize_wma_emulator();

static int netinit = 0;
/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_start(void) {

    static WSADATA wsaData;

    if (!netinit) {
        lastError = 0;
        rootAddrContext.asyncHandle = 0;
        rootAddrContext.next = NULL;

        if (0 != WSAStartup(0x0101, &wsaData )) {
            return JAVACALL_FAIL;
        }

        netinit = 1;
    }

#ifdef ENABLE_JSR_120
    init_wma_emulator();
#endif
#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_init_start\n");
#endif
    return JAVACALL_OK;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_finish() {

#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_init_finish\n");
#endif
    return JAVACALL_OK;
}

/**
 * Performs platform-specific finalization of the networking system.
 * Will be called ONCE during VM shutdown.
 *
 * @retval JAVACALL_WOULD_BLOCK caller must call xxx_finalize_finish
 *         function to complete the operation
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_start(void)
{
#ifdef ENABLE_JSR_120
    finalize_wma_emulator();
#endif
#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_finalize_start\n");
#endif
    return JAVACALL_OK;
}

/**
 * Finishes platform-specific finalize of the networking system.
 * The function is invoked be the JVM after receiving JAVACALL_NETWORK_DOWN
 * notification from the platform.
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_finish(void)
{
#ifdef ENABLE_NETWORK_TRACING
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, "network_finalize_finish\n");
#endif
    return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_network_error(void *handle){

    (void)handle;
    return lastError;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_get_local_host_name(char *pLocalHost){

    javacall_result res;

    if(netinit == 0) {
       res = javacall_network_init_start();
       if(res != JAVACALL_OK) {
          return JAVACALL_FAIL;
       }
    }
    if (SOCKET_ERROR == gethostname(pLocalHost, MAX_HOST_LENGTH)) {
        return JAVACALL_FAIL;
    }
    return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_open_start(
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
	int port, 
	void **pHandle, 
	void **pContext) 
{
    SOCKET s;
    int falsebuf = 0;
    int status;
    struct sockaddr_in addr;
    unsigned long nonBlockingFlag = 1;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == s) {

        return JAVACALL_FAIL;
    }

    status = setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                        (char*)&falsebuf, sizeof (falsebuf));
    lastError = WSAGetLastError();
    if (SOCKET_ERROR == status) {
        (void)closesocket(s);

		return JAVACALL_FAIL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    status = bind(s, (struct sockaddr*)&addr, sizeof(addr));

	if (status== SOCKET_ERROR) {
	      (void)closesocket(s);
          return JAVACALL_FAIL;
    } else { //bind OK
	      status = listen(s, 3);
	      *pHandle = (void*)s;   
		 
#ifdef ENABLE_NETWORK_TRACING
          sprintf(print_buffer,"listen(%d) = %d\n", pHandle, status);
          javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, print_buffer);
#endif 

          if(status == SOCKET_ERROR) {
              closesocket(s);
              return JAVACALL_FAIL;
          }
	     //make socket non-blocking
            status = ioctlsocket(s, FIONBIO, &nonBlockingFlag);

            if(status ==SOCKET_ERROR) {
	         closesocket(s);
             return JAVACALL_FAIL;
            }

            WSAAsyncSelect(s, getPhantomWindowHandle(), WM_NETWORK, FD_ACCEPT);
            return JAVACALL_OK;  //listen OK	     
        }//bind OK         
    
    return JAVACALL_CONNECTION_NOT_FOUND; 
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_server_socket_open_finish(void *handle,void *context){

	return JAVACALL_OK;
}



static int  winsock_server_socket_accept_common(
     javacall_handle handle, 
     javacall_handle *newhandle) {
    
   SOCKET connfd;
   struct sockaddr sa;
   int saLen = sizeof (sa);

   connfd = accept((SOCKET)handle, &sa, &saLen);
#ifdef ENABLE_NETWORK_TRACING
    sprintf(print_buffer,"accept(%d) = %d\n", handle, connfd);
    javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_NETWORK, print_buffer);
#endif
    if (connfd == SOCKET_ERROR) {
        int lastError = WSAGetLastError();

        if (lastError == WSAEWOULDBLOCK) {          
            WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), WM_NETWORK,
                   FD_ACCEPT);            
            return JAVACALL_WOULD_BLOCK;
        } else if (lastError == WSAEINTR || lastError == WSAENOTSOCK) {
            WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), 0, 0);
            return JAVACALL_INTERRUPTED;
        } else {
            WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), 0, 0);
            return JAVACALL_FAIL;
        }
    }
    
   *newhandle = (javacall_handle)connfd;
    WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), 0, 0);    
    return JAVACALL_OK;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_start(
      javacall_handle handle, 
      javacall_handle *pNewhandle) {
   
    return winsock_server_socket_accept_common( handle,pNewhandle);     
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_finish(
	javacall_handle handle, 
	javacall_handle *pNewhandle) {

   return winsock_server_socket_accept_common( handle, pNewhandle);      
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_server_socket_set_notifier(javacall_handle handle, javacall_bool set) {
	int res;

    if (JAVACALL_TRUE == set) {
        res = WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), WM_NETWORK, FD_ACCEPT);
    } else {
        res = WSAAsyncSelect((SOCKET)handle, getPhantomWindowHandle(), 0, 0);
    }
	
	if (0 == res)
		return JAVACALL_OK;
	else
		return JAVACALL_FAIL;
}

/**
 * Gets the http / https proxy address. This method is
 * called when the <tt>com.sun.midp.io.http.proxy</tt> or 
 <tt>com.sun.midp.io.https.proxy</tt> internal property
 * is retrieved.
 *
 * @param pHttpProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpProxy should be (MAX_HOST_LENGTH + 6).
 * @param pHttpsProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpsProxy should be (MAX_HOST_LENGTH + 6).
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */
javacall_result /*OPTIONAL*/ javacall_network_get_http_proxy(/*OUT*/ char *pHttpProxy) {
    if (NULL != pHttpProxy) {
        int length = GetEnvironmentVariable("http_proxy", pHttpProxy, MAX_HOST_LENGTH + 7);

        if (0 == length || MAX_HOST_LENGTH + 7 < length) {
            return JAVACALL_FAIL;
        }
    }
/*
    if (NULL != pHttpsProxy) {
        int length = GetEnvironmentVariable("https_proxy", pHttpsProxy, MAX_HOST_LENGTH + 7);

        if (0 == length) {
            length = GetEnvironmentVariable("http_proxy", pHttpsProxy, MAX_HOST_LENGTH + 7);
        }
        
        if (0 == length || MAX_HOST_LENGTH + 7 < length) {
            return JAVACALL_FAIL;
        }
    }
*/
    return JAVACALL_OK;
}

/**
 * Gets the port number of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocalport(
        javacall_handle handle,
        int *pPortNumber) {

    SOCKET s = (SOCKET)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getsockname(s, (struct sockaddr*)&sa, &saLen);
    if (SOCKET_ERROR == status) {
        return JAVACALL_FAIL;
    }

    *pPortNumber = ntohs(sa.sin_port);

    return JAVACALL_OK;
}

/**
 * Gets the port number of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result javacall_socket_getremoteport(
    void *handle,
    int *pPortNumber)
{
    SOCKET s = (SOCKET)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getpeername(s, (struct sockaddr*)&sa, &saLen);
    if (SOCKET_ERROR == status) {
        return JAVACALL_FAIL;
    }

    *pPortNumber = ntohs(sa.sin_port);

    return JAVACALL_OK;
}

/**
 * Gets the IP address of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocaladdr(
    javacall_handle handle,
    char *pAddress)
{
    SOCKET s = (SOCKET)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getsockname(s, (struct sockaddr*)&sa, &saLen);
    lastError = WSAGetLastError();
    if (SOCKET_ERROR == status) {
        return JAVACALL_FAIL;
    }

    /*
     * The string returned from inet_ntoa is only valid until the next
     * winsock API call. The string should not be freed.
     */
    strcpy(pAddress, inet_ntoa(sa.sin_addr));

    return JAVACALL_OK;
}

/**
 * Gets the IP address of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getremoteaddr(
    void *handle,
    char *pAddress)
{
    SOCKET s = (SOCKET)handle;
    int status;
    struct sockaddr_in sa;
    int saLen = sizeof (sa);

    sa.sin_family = AF_INET;
    status = getpeername(s, (struct sockaddr*)&sa, &saLen);
    if (SOCKET_ERROR == status) {
        return JAVACALL_FAIL;
    }

    /*
     * The string returned from inet_ntoa is only valid until the next
     * winsock API call. The string should not be freed.
     */
    strcpy(pAddress, inet_ntoa(sa.sin_addr));

    return JAVACALL_OK;
}

char* javacall_inet_ntoa(javacall_ip_version ip_version, void *address) {
    return inet_ntoa(*((struct in_addr*)address));
}

extern javacall_result try_process_wma_emulator(javacall_handle handle);

static int handleNetworkStreamEvents(WPARAM wParam,LPARAM lParam) {
    switch(WSAGETSELECTEVENT(lParam)) {
    case FD_CONNECT:
            /* Change this to a write. */
        javanotify_socket_event(
                               JAVACALL_EVENT_SOCKET_CONNECT_COMPLETED,
                               (javacall_handle)wParam,
                               (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "FD_CONNECT)\n");
#endif
        return 0;
    case FD_WRITE:
        javanotify_socket_event(
                               JAVACALL_EVENT_SOCKET_SEND,
                               (javacall_handle)wParam,
                               (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "FD_WRITE)\n");
#endif
        return 0;

    case FD_ACCEPT:
        {
            //accept
            SOCKET clientfd = 0;
            javanotify_server_socket_event(
                                          JAVACALL_EVENT_SERVER_SOCKET_ACCEPT_COMPLETED,
                                          (javacall_handle)wParam,
                                          (javacall_handle)clientfd,
                                          (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
            fprintf(stderr, "FD_ACCEPT, ");
#endif
        }
    case FD_READ:
        javanotify_socket_event(
                               JAVACALL_EVENT_SOCKET_RECEIVE,
                               (javacall_handle)wParam,
                               (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "FD_READ)\n");
#endif
        return 0;

    case FD_CLOSE:
		{
		javacall_result ret;
		switch (WSAGETSELECTERROR(lParam)) {
		case WSAEINTR:
			ret = JAVACALL_INTERRUPTED;
			break;
		case 0:
			ret = JAVACALL_OK;
			break;
		default:
			ret = JAVACALL_FAIL;
		}
        javanotify_socket_event(
                               JAVACALL_EVENT_SOCKET_CLOSE_COMPLETED,
                               (javacall_handle)wParam,
                                ret);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "FD_CLOSE)\n");
#endif
        return 0;
    	}
    default:
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "unsolicited event %d)\n",
                WSAGETSELECTEVENT(lParam));
#endif
        break;
    }
    return 0;
}

static int handleNetworkDatagramEvents(WPARAM wParam,LPARAM lParam) {
    switch(WSAGETSELECTEVENT(lParam)) {

    case FD_WRITE:
        javanotify_datagram_event(
                                 JAVACALL_EVENT_DATAGRAM_SENDTO_COMPLETED,
                                 (javacall_handle)wParam,
                                 (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "[UDP] FD_WRITE)\n");
#endif
        return 0;
    case FD_READ:
#ifdef ENABLE_JSR_120
        if (JAVACALL_FALSE != try_process_wma_emulator((javacall_handle)wParam)) {
            return 0;
        }
#endif

        javanotify_datagram_event(
                                 JAVACALL_EVENT_DATAGRAM_RECVFROM_COMPLETED,
                                 (javacall_handle)wParam,
                                 (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "[UDP] FD_READ)\n");
#endif
        return 0;
    case FD_CLOSE:
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "[UDP] FD_CLOSE)\n");
#endif
        return 0;
    default:
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "[UDP] unsolicited event %d)\n",
                WSAGETSELECTEVENT(lParam));
#endif
        break;
    }//end switch
    return 0;
}


static LRESULT CALLBACK
WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    //check if udp or tcp
    int level = SOL_SOCKET;
    int opttarget;
    int optname;
    int optsize = sizeof(optname);

    switch(iMsg) {

    case WM_CREATE:
        return 0;


    case WM_CLOSE:
        /*
         * Handle the "X" (close window) button by sending the AMS a shutdown
         * event.
         */
        //javanotify_shutdown();
        PostQuitMessage(0);
        return 0;

    case WM_NETWORK:

#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "Got WM_NETWORK(");
        fprintf(stderr, "descriptor = %d, ", (int)wParam);
        fprintf(stderr, "status = %d, ", WSAGETSELECTERROR(lParam));
#endif
        optname = SO_TYPE;
        if(0 != getsockopt((SOCKET)wParam, SOL_SOCKET,  optname,(char*)&opttarget, &optsize)) {
#ifdef ENABLE_NETWORK_TRACING
            fprintf(stderr, "getsocketopt error)\n");
#endif
        }

        if(opttarget == SOCK_STREAM) { // TCP socket

            return handleNetworkStreamEvents(wParam,lParam);

        } else {
            return handleNetworkDatagramEvents(wParam,lParam);
        };

        break;

    case WM_HOST_RESOLVED:
#ifdef ENABLE_NETWORK_TRACING
        fprintf(stderr, "Got Windows event WM_HOST_RESOLVED \n");
#endif
        javanotify_socket_event(
                               JAVACALL_EVENT_NETWORK_GETHOSTBYNAME_COMPLETED,
                               (javacall_handle)wParam,
                               (WSAGETSELECTERROR(lParam) == 0) ? JAVACALL_OK : JAVACALL_FAIL);
        return 0;

#if 0
    case WM_DEBUGGER:
        pSignalResult->waitingFor = VM_DEBUG_SIGNAL;
        return 0;
#endif
    default:
        break;
    } /* end of external switch */

    return DefWindowProc (hwnd, iMsg, wParam, lParam);

} /* end of WndProc */

static void InitializePhantomWindow() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    char szAppName[] = "no_window";
    WNDCLASSEX  wndclass;
    HWND hwnd;

    wndclass.cbSize        = sizeof (wndclass);
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.hIconSm       = NULL;

    RegisterClassEx (&wndclass);

    hwnd = CreateWindow (szAppName,               /* window class name       */
                         "JavaVM",                  /* window caption          */
                         WS_DISABLED,             /* window style; disable   */
                         CW_USEDEFAULT,           /* initial x position      */
                         CW_USEDEFAULT,           /* initial y position      */
                         0,                       /* initial x size          */
                         0,                       /* initial y size          */
                         NULL,                    /* parent window handle    */
                         NULL,                    /* window menu handle      */
                         hInstance,               /* program instance handle */
                         NULL);                   /* creation parameters     */

    hPhantomWindow = hwnd;
}

HWND getPhantomWindowHandle() {
    if(hPhantomWindow == NULL) {
    	InitializePhantomWindow();
    }
    return hPhantomWindow;
}

