/*
 *	websSSL.c -- MatrixSSL Layer

 *
 *	(C)Copyright 2002-2010 PeerSec Networks
 *	All Rights Reserved
 *
 */
/******************************************************************************/

#ifdef WEBS_SSL_SUPPORT

/******************************** Description *********************************/
/*
 *	GoAhead Webs HTTPS implementation uses PeerSec Networks MatrixSSL
 */

/********************************* Includes ***********************************/

#include "wsIntrn.h"
#include "websSSL.h"
#include "webs.h"
#include "matrixSSLSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************* Definitions **********************************/

static int			sslListenSock   = -1;            /* Listen socket */
static sslKeys_t	*sslKeys = NULL;

/******************************* Prototypes  **********************************/

static int	websSSLAccept(int sid, char *ipaddr, int port, int listenSid);
static void	websSSLSocketEvent(int sid, int mask, void *iwp);
static int	websSSLReadEvent(webs_t wp);

/******************************************************************************/
/*
 *    Start up the SSL Context for the application, and start a listen on the
 *    SSL port (usually 443, and defined by WEBS_DEFAULT_SSL_PORT)
 *    Return 0 on success, -1 on failure.
 *
 *    Not freeing keys or calling matrixSslClose on failures because
 *    websSSLClose will be called at application close to do that
 */
int websSSLOpen()
{
	if (matrixSslOpen() < 0) {
		return -1;
    }
	
	if (matrixSslNewKeys(&sslKeys) < 0) {
		trace(0, T("Failed to allocate keys in websSSLOpen\n"));
		return -1;
	}
	if (matrixSslLoadRsaKeys(sslKeys, DEFAULT_CERT_FILE, DEFAULT_KEY_FILE, 
							  NULL /* privPass */,  
							  NULL /* trustedCAFile */) < 0) {
		trace(0, T("Failed to read certificate %s in websSSLOpen\n"),
			DEFAULT_CERT_FILE);
		trace(0, T("SSL support is disabled\n"));
		return -1;
    }

#ifdef USE_NONBLOCKING_SSL_SOCKETS
	sslListenSock = socketOpenConnection(NULL, WEBS_DEFAULT_SSL_PORT, 
		websSSLAccept, SOCKET_MYOWNBUFFERS);
#else
	sslListenSock = socketOpenConnection(NULL, WEBS_DEFAULT_SSL_PORT, 
		websSSLAccept, SOCKET_BLOCK | SOCKET_MYOWNBUFFERS);
#endif
	
	if (sslListenSock < 0) {
		trace(0, T("SSL: Unable to open SSL socket on port %d.\n"), 
				WEBS_DEFAULT_SSL_PORT);
		return -1;
    }
	trace(0, T("webs: Listening for HTTPS requests at port %d\n"),
		WEBS_DEFAULT_SSL_PORT);
	
    return 0;
}

/******************************************************************************/
/*
 *    Accept a connection
 */
int websSSLAccept(int sid, char *ipaddr, int port, int listenSid)
{
    webs_t    wp;
    int        wid;
	
    a_assert(ipaddr && *ipaddr);
    a_assert(sid >= 0);
    a_assert(port >= 0);
	
	/*
	 *    Allocate a new handle for this accepted connection. This will allocate
	 *    a webs_t structure in the webs[] list
	 */
    if ((wid = websAlloc(sid)) < 0) {
        return -1;
    }
    wp = webs[wid];
    a_assert(wp);
    wp->listenSid = listenSid;
	
    ascToUni(wp->ipaddr, ipaddr, min(sizeof(wp->ipaddr), strlen(ipaddr)+1));
	
	/*
	 *	Check if this is a request from a browser on this system. This is useful
	 *	to know for permitting administrative operations only for local access
	 */
    if (gstrcmp(wp->ipaddr, T("127.0.0.1")) == 0 ||
		gstrcmp(wp->ipaddr, websIpaddr) == 0 ||
		gstrcmp(wp->ipaddr, websHost) == 0) {
        wp->flags |= WEBS_LOCAL_REQUEST;
    }
	/*
	 *	Since the acceptance came in on this channel, it must be secure
	 */
    wp->flags |= WEBS_SECURE;
	
	/*
	 *	Arrange for websSocketEvent to be called when read data is available
	 */
    socketCreateHandler(sid, SOCKET_READABLE, websSSLSocketEvent, wp);
	
	/*
	 *	Arrange for a timeout to kill hung requests
	 */
    wp->timeout = emfSchedCallback(WEBS_TIMEOUT, websTimeout, (void *) wp);
    trace(8, T("websSSLAccept(): webs: accept request\n"));
    return 0;
}

/******************************************************************************/
/*
 *    Perform a read of the SSL socket
 */
int websSSLRead(websSSL_t *wsp, char_t *buf, int len)
{
	a_assert(wsp);
	a_assert(buf);
	
	return sslRead(wsp->sslConn, buf, len);
}

/******************************************************************************/
/*
 *    Perform a gets of the SSL socket, returning an balloc'ed string
 *
 *    Get a string from a socket. This returns data in *buf in a malloced string
 *    after trimming the '\n'. If there is zero bytes returned, *buf will be set
 *    to NULL. If doing non-blocking I/O, it returns -1 for error, EOF or when
 *    no complete line yet read. If doing blocking I/O, it will block until an
 *    entire line is read. If a partial line is read socketInputBuffered or
 *    socketEof can be used to distinguish between EOF and partial line still
 *    buffered. This routine eats and ignores carriage returns.
 */
int websSSLGets(websSSL_t *wsp, char_t **buf)
{ 
	socket_t    *sp;
	ringq_t       *lq;
	char        c;
	int         len;
	webs_t      wp;
	int         sid;
	int         numBytesReceived;
	
	a_assert(wsp);
	a_assert(buf);
	
	*buf = NULL;
	
	wp  = wsp->wp;
	sid = wp->sid;
	
	if ((sp = socketPtr(sid)) == NULL) {
		return -1;
	}
	lq = &sp->lineBuf;
	
	while (1) {
		
		/* read one byte at a time */
		numBytesReceived = sslRead(wsp->sslConn, &c, 1);
		
		if (numBytesReceived < 0) {
			return -1;
		}
		
		if (numBytesReceived == 0) {
			/*
			 * If there is a partial line and we are at EOF, pretend we saw a '\n'
			 */
			if (ringqLen(lq) > 0 && (sp->flags & SOCKET_EOF)) {
				c = '\n';
			} else {
				len = ringqLen(lq);
				if (len > 0) {
					*buf = ballocAscToUni((char *)lq->servp, len);
				} else {
					*buf = NULL;
				}
				ringqFlush(lq);
				return len;
			}
		}
		/*
		 * If a newline is seen, return the data excluding the new line to the
		 * caller. If carriage return is seen, just eat it.
		 */
		if (c == '\n') {
			len = ringqLen(lq);
			if (len > 0) {
				*buf = ballocAscToUni((char *)lq->servp, len);
			} else {
				*buf = NULL;
			}
			ringqFlush(lq);
			return len;
			
		} else if (c == '\r') {
			continue;
		}
		ringqPutcA(lq, c);
	}
	return 0;
}

/******************************************************************************/
/*
 *    The webs socket handler.  Called in response to I/O. We just pass control
 *    to the relevant read or write handler. A pointer to the webs structure
 *    is passed as a (void *) in iwp.
 */
static void websSSLSocketEvent(int sid, int mask, void *iwp)
{
    webs_t    wp;
	
    wp = (webs_t) iwp;
    a_assert(wp);
	
    if (! websValid(wp)) {
        return;
    }
	
    if (mask & SOCKET_READABLE) {
        websSSLReadEvent(wp);
    }
    if (mask & SOCKET_WRITABLE) {
        if (wp->writeSocket) {
            (*wp->writeSocket)(wp);
        }
    }
}

/******************************************************************************/
/*
 *    Handler for SSL Read Events
 */
static int websSSLReadEvent (webs_t wp)
{
    int ret = 07, sock, resume;
    socket_t *sptr;
    sslConn_t* sslConn;
	
    a_assert (wp);
    a_assert(websValid(wp));
	
    sptr = socketPtr(wp->sid);
    a_assert(sptr);
	
    sock = sptr->sock;
	
	if (wp->wsp == NULL) {
		resume = 0;
	} else {
		resume = 1;
		sslConn = (wp->wsp)->sslConn;
	}
/*
	This handler is essentially two back-to-back calls to sslRead.  The
	first is here in sslAccept where the handshake is to take place.  The
	second is in websReadEvent below where it is expected the client was
	contacting us to send an HTTP request and we need to read that off.
	
	The introduction of non-blocking sockets has made this a little more
	confusing becuase it is possible to return from either of these sslRead 
	calls in a WOULDBLOCK state.  It doesn't ultimately matter, however,
	because sslRead is fully stateful so it doesn't matter how/when it gets
	invoked later (as long as the correct sslConn is used (which is what the
	resume variable is all about))
*/
    if (sslAccept(&sslConn, sock, sslKeys, resume, NULL) < 0) {
		websTimeoutCancel(wp);
		socketCloseConnection(wp->sid);
		websFree(wp);
		return -1;
    }
	
	if (resume == 0) {
	/*
	 *    Create the SSL data structure in the wp.
	 */
		wp->wsp = balloc(B_L, sizeof(websSSL_t));
		a_assert (wp->wsp);
	
		(wp->wsp)->sslConn = sslConn;
		(wp->wsp)->wp = wp;
	}
	
	/*
	 *    Call the default Read Event
	 */
    websReadEvent(wp);
	
    return ret;
}

/******************************************************************************/
/*
 *    Return TRUE if websSSL has been opened
 */
int websSSLIsOpen()
{
    return (sslListenSock != -1);
}

/******************************************************************************/
/*
 *    Perform a write to the SSL socket
 */
int websSSLWrite(websSSL_t *wsp, char_t *buf, int len)
{
	int sslBytesSent = 0;
	
	a_assert(wsp);
	a_assert(buf);
	
	if (wsp == NULL) {
		return -1;
	}
/*
	Send on socket. If non-blocking, 0 may be returned from this call,
	indicating that data could not be sent due to EWOULDBLOCK.
 */
	if ((sslBytesSent = sslWrite(wsp->sslConn, buf, len)) < 0) {
		sslBytesSent = -1;
	}
	return sslBytesSent;
}

/******************************************************************************/
/*
 *    Return Eof for the underlying socket
 */
int websSSLEof(websSSL_t *wsp)
{
    webs_t      wp;
    int         sid;
	
    a_assert(wsp);
	
    wp  = wsp->wp;
    sid = wp->sid;
	
    return socketEof(sid);
}

/******************************************************************************/
/*
 *   Flush stub for compatibility
 */
int websSSLFlush(websSSL_t *wsp)
{
    a_assert(wsp);
	
    /* Autoflush - do nothing */
    return 0;
}

/******************************************************************************/
/*
 *    Free SSL resources
 */
int websSSLFree(websSSL_t *wsp)
{
    if (wsp != NULL) {
		sslWriteClosureAlert(wsp->sslConn);
		sslFreeConnection(&wsp->sslConn);
		bfree(B_L, wsp);
    }
    return 0;
}

/******************************************************************************/
/*
 *    Stops the SSL system
 */
void websSSLClose()
{
	matrixSslDeleteKeys(sslKeys);
	matrixSslClose();
}

/******************************************************************************/

#endif /* WEBS_SSL_SUPPORT */
