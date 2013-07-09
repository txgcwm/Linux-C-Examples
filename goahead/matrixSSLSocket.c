/*
 *	matrixSSLSocket.c -	SSL socket layer for MatrixSSL
 *	Release $Name: WEBSERVER-2-5 $
 *
 *	(C)Copyright 2002-2010 PeerSec Networks
 *	All Rights Reserved
 */
/******************************************************************************/

#ifdef WEBS_SSL_SUPPORT

#include <stdlib.h>
#include <stdio.h>
#include "matrixSSLSocket.h"
#include "uemf.h"

#ifdef MACOSX
#define OSX 1
#define LINUX 1
#endif

#define	MAX_WRITE_MSEC	500	/* Fail if we block more than X millisec on write */

/******************************************************************************/
/*
	Socket layer API for the MatrixSSL library.  
*/
static int waitForWriteEvent(int fd, int msec);
static void setSocketNonblock(SOCKET sock);

/******************************************************************************/
/*
	Server side.  Accept an incomming SSL connection request.
	'conn' will be filled in with information about the accepted ssl connection

	return -1 on error, 0 on success, or WOULD_BLOCK for non-blocking sockets
*/
int sslAccept(sslConn_t **conn, SOCKET fd, sslKeys_t *keys, int32 resume,
			  int32 (*certValidator)(ssl_t *, psX509Cert_t *, int32))
{
	sslConn_t		*cp;
	int				rc;
	
	if (resume == 0) {
/*
		Associate a new ssl session with this socket.  The session represents
		the state of the ssl protocol over this socket.  Session caching is
		handled automatically by this api.
*/
		cp = balloc(B_L, sizeof(sslConn_t));
		memset(cp, 0x0, sizeof(sslConn_t));
		cp->fd = fd;
		if (matrixSslNewServerSession(&cp->ssl, keys, certValidator) < 0) {
			sslFreeConnection(&cp);
			return -1;
		}
#ifdef USE_NONBLOCKING_SSL_SOCKETS
		/* Set this ourselves, "just to make sure" */
		setSocketNonblock(fd);
#endif
	} else {
		cp = *conn;
	}
/*
	This call will perform the SSL handshake
*/
	rc = sslRead(cp, NULL, 0);
	if (rc < 0) {
		return -1;
	}
	*conn = cp;
	return 0;
}
	
/******************************************************************************/	
/*
	Primary MatrixSSL read function that transparently handles SSL handshakes
	and the subsequent incoming application data records.
	
	A NULL inbuf parameter is an indication that this is a call to perform a
	new SSL handshake with an incoming client and no out data is expected
	
	Params:
	inbuf	allocated storage for plaintext data to be copied to
	inlen	length of inbuf
	
	Return codes:
	-1	EOF or internal failure.  caller should free sess and close socket
	0	success status.  no data is being returned to the caller
	>0	success status.  number of plaintext bytes written to inbuf
 
	Note that unlike a standard "socket read", a read of an SSL record can
	produce data that must be written, for example, a response to a handshake
	message that must be sent before any more data is read. Also, data can 
	be read from the network such as an SSL alert, that produces no data to
	pass back to caller.
 
	Because webs doesn't have the concept of a read forcing a write, we
	do the write here, inline. If we are non-blocking, this presents an issue
	because we can't block indefinitely on a send, and we also can't indicate 
	that the send be done later. The workaround below uses select() to 
	implement a "timed send", which will fail and indicate the connection be
	closed if not complete within a time threshold. 
 
	This situation is extremely unlikely in normal operation, since the only 
	records that must be sent as a result of a recv are handshake messages, 
	and TCP buffers would not typically be full enough at that point to 
	result in an EWOULDBLOCK on send. Conceivably, it could occur on a client
	initiated SSl re-handshake that is sent by a slow-reading client to a 
	server with full TCP buffers. A non-malicious client in this situation
	would read immediately after a re-handshake request and fail anyway
	because a buffered appdata record would be read.
*/
int	sslRead(sslConn_t *cp, char *inbuf, int inlen)
{	
	unsigned char	*buf;
	int				rc, len, transferred;

/*
	Always first look to see if any plaintext application data is waiting 
	We have two levels of buffer here, one for decoded data and one for
	partial, still encoded SSL records. The partial SSL records are stored
	transparently in MatrixSSL, but the plaintext is stored in 'cp'.
 */
	if (inbuf != NULL && inlen > 0 && cp->ptBytes > 0 && cp->pt) {
		if (cp->ptBytes < inlen) {
			inlen = cp->ptBytes;
		}
		memcpy(inbuf, cp->currPt, inlen);
		cp->currPt += inlen;
		cp->ptBytes -= inlen;
		/* Free buffer as we go if empty */
		if (cp->ptBytes == 0) {
			bfree(B_L, cp->pt);
			cp->pt = cp->currPt = NULL;
		}
		return inlen;
	}
/*
	If there is outgoing data buffered, just try to write it here before
	doing our read on the socket. Because the read could produce data to 
	be written, this will ensure we have as much room as possible in that
	case for the written record. Note that there may be data here to send
	because of a previous sslWrite that got EWOULDBLOCK.
*/
WRITE_MORE:
	if ((len = matrixSslGetOutdata(cp->ssl, &buf)) > 0) {
		transferred = send(cp->fd, buf, len, MSG_NOSIGNAL);
		if (transferred <= 0) {
			if (socketGetError() != EWOULDBLOCK) {
				return -1;
			}
			if (waitForWriteEvent(cp->fd, MAX_WRITE_MSEC) == 0) {
				goto WRITE_MORE;
			}
			return -1;
		} else {
			/* Indicate that we've written > 0 bytes of data */
			if ((rc = matrixSslSentData(cp->ssl, transferred)) < 0) {
				return -1;
			}
			if (rc == MATRIXSSL_REQUEST_CLOSE) {
				return -1;
			} else if (rc == MATRIXSSL_HANDSHAKE_COMPLETE) {
				/* If called via sslAccept (NULL buf), then we can just leave */
				return 0;
			}
			/* Try to send again if more data to send */
			if (rc == MATRIXSSL_REQUEST_SEND || transferred < len) {
				goto WRITE_MORE;
			}
		}
	} else if (len < 0) {
		return -1;
	}

READ_MORE:
	/* Get the ssl buffer and how much data it can accept */
	/* Note 0 is a return failure, unlike with matrixSslGetOutdata */
	if ((len = matrixSslGetReadbuf(cp->ssl, &buf)) <= 0) {
		return -1;
	}
	if ((transferred = recv(cp->fd, buf, len, MSG_NOSIGNAL)) < 0) {
		/* Support non-blocking sockets if turned on */
		if (socketGetError() == EWOULDBLOCK) {
				return 0;
		}
		trace(1, T("RECV error: %d\n"), socketGetError());
		return -1;
	}
	if (transferred == 0) {
		/* If EOF, remote socket closed. This is semi-normal closure. */	
		trace(4, T("Closing connection %d on EOF\n"), cp->fd);
		return -1;
	}
/*
	Notify SSL state machine that we've received more data into the
	ssl buffer retreived with matrixSslGetReadbuf.
 */
	if ((rc = matrixSslReceivedData(cp->ssl, transferred, &buf, 
			(uint32*)&len)) < 0) {
		return -1;
	}

PROCESS_MORE:		
	switch (rc) {
		case MATRIXSSL_REQUEST_SEND:
			/* There is a handshake response we must send */
			goto WRITE_MORE;
		case MATRIXSSL_REQUEST_RECV:
			goto READ_MORE;
		case MATRIXSSL_HANDSHAKE_COMPLETE:
			/* Session resumption handshake */
			goto READ_MORE;
		case MATRIXSSL_RECEIVED_ALERT:
			/* Any fatal alert will simply cause a read error and exit */
			if (*buf == SSL_ALERT_LEVEL_FATAL) {
				trace(1, T("Fatal alert: %d, closing connection.\n"), 
							*(buf + 1));
				return -1;
			}
			/* Closure alert is normal (and best) way to close */
			if (*(buf + 1) == SSL_ALERT_CLOSE_NOTIFY) {
				return -1;
			}
			/* Eating warning alerts */
			trace(4, T("Warning alert: %d\n"), *(buf + 1));
			if ((rc = matrixSslProcessedData(cp->ssl, &buf, (uint32*)&len))
					== 0) {
				/* Possible there was plaintext before the alert */
				if (inbuf != NULL && inlen > 0 && cp->ptBytes > 0 && cp->pt) {
					if (cp->ptBytes < inlen) {
						inlen = cp->ptBytes;
					}
					memcpy(inbuf, cp->currPt, inlen);
					cp->currPt += inlen;
					cp->ptBytes -= inlen;
					/* Free buffer as we go if empty */
					if (cp->ptBytes == 0) {
						bfree(B_L, cp->pt);
						cp->pt = cp->currPt = NULL;
					}
					return inlen;
				} else {
					return 0;
				}
			}
			goto PROCESS_MORE;

		case MATRIXSSL_APP_DATA:
			if (cp->ptBytes == 0) {
/*
				Catching here means this is new app data just grabbed off the
				wire. 
*/
				cp->ptBytes = len;
				cp->pt = balloc(B_L, len);
				memcpy(cp->pt, buf, len);
				cp->currPt = cp->pt;
			} else {
/*
				Multi-record.  This case should only ever be possible if no
				data has already been read out of the 'pt' cache so it is
				fine to assume an unprocessed buffer.
*/
				psAssert(cp->pt == cp->currPt);
				cp->pt = brealloc(B_L, cp->pt, cp->ptBytes + len);
				memcpy(cp->pt + cp->ptBytes, buf, len);
				cp->currPt = cp->pt;
				cp->ptBytes += len;
			}
			if ((rc = matrixSslProcessedData(cp->ssl, &buf, (uint32*)&len))
					< 0) {
				return -1;
			}
			/* Check for multi-record app data*/
			if (rc > 0) {
				goto PROCESS_MORE;
			}
/*
			Otherwise pass back how much the caller wants to read (if any)
*/
			if (inbuf != 0 && inlen > 0) {
				if (cp->ptBytes < inlen) {
					inlen = cp->ptBytes;
				}
				memcpy(inbuf, cp->currPt, inlen);
				cp->currPt += inlen;
				cp->ptBytes -= inlen;
				return inlen; /* Just a breakpoint holder */
			}
			return 0; /* Have it stored, but caller didn't want any data */
		default:
			return -1;
	}

	return 0; /* really can never hit this */
}

/******************************************************************************/
/*
	sslWrite encodes 'data' as a single SSL record.

	Return codes:
	-1	Internal failure. caller should free sess and close socket
	0	WOULDBLOCK. caller should call back with same input later
	> 0	success status.  number of plaintext bytes encoded and sent
 
	In order for non-blocking sockets to work transparently to the upper
	layers in webs, we manage the buffering of outgoing data and fudge the
	number of bytes sent until the entire SSL record is sent.
 
	This is because the encoded SSL record will be longer than 'len' with the 
	SSL header prepended and the MAC and padding appended. There is no easy way
	to indicate to the caller if only a partial record was sent via the webs
	socket API.

	For example, if the caller specifies one byte of 'data' ('len' == 1),
	the SSL record could be 33 bytes long. If the socket send is able to write
	10 bytes of data, how would we indicate this to the caller, which expects
	only 1 byte, 0 bytes or a negative error code as a result of this call?
 
	The solution here is to always return 0 from this function, until we have
	flushed out all the bytes in the SSL record (33 in this example), and when
	the record has all been sent, return the originally requested length, which
	is 1 in this example.
 
	This assumes that on a 0 return, the caller will wait for a writable event 
	on the socket and re-call this api with the same 'data' and 'len' as 
	previously sent. Essentially a 0 return means "retry the same sslWrite call
	later".
*/
int sslWrite(sslConn_t *cp, char *data, int len)
{
	unsigned char	*buf; 
	int				rc, transferred, ctLen;
/*
	If sendBlocked is set, then the previous time into sslWrite with this cp
	could not send all the requested data, and zero was returned to the caller.
	In this case, the data has already been encoded into the SSL outdata buffer,
	and we don't need to re-encode it here.
	This assumes that the caller is sending the same 'len' and contents of
	'data' as they did last time.
 */
	if (!cp->sendBlocked) {
		if (matrixSslGetWritebuf(cp->ssl, &buf, len) < len) { 
			/* SSL buffer must hold requested plaintext + SSL overhead */
			return -1;
		}
		memcpy((char *)buf, data, len); 
		if (matrixSslEncodeWritebuf(cp->ssl, len) < 0) { 
			return -1; 
		}
	} else {
/*
		Not all previously encoded data could be sent without blocking and 
		0 bytes was previously returned to caller as sent. Ensure caller is
		retrying with same len (and presumably the same data).
 */
		if (len != cp->ptReqBytes) {
			a_assert(len != cp->ptReqBytes);
			return -1;
		}
	}
WRITE_MORE:
/*
	There is a small chance that we are here with sendBlocked set, and yet
	there is no buffered outdata to send. This happens because a send can 
	also happen in sslRead, and that could have flushed the outgoing buffer
	in addition to a handshake message reply.
 */
	ctLen = matrixSslGetOutdata(cp->ssl, &buf);
	if (ctLen > 0) {
		transferred = send(cp->fd, buf, ctLen, MSG_NOSIGNAL); 
		if (transferred <= 0) {
#ifdef USE_NONBLOCKING_SSL_SOCKETS
			if (socketGetError() != EWOULDBLOCK) {
				return -1;
			}
			if (!cp->sendBlocked) {
				cp->sendBlocked = 1;
				cp->ptReqBytes = len;
			} else {
			}
			return 0;
#else
			return -1;
#endif
		}
		/* Update the SSL buffer that we've written > 0 bytes of data */ 
		if ((rc = matrixSslSentData(cp->ssl, transferred)) < 0) { 
			return -1; 
		}
		/* There is more data in the SSL buffer to send */
		if (rc == MATRIXSSL_REQUEST_SEND) {
			goto WRITE_MORE;
		}
	}
	cp->sendBlocked = 0;
	cp->ptReqBytes = 0;
	return len; 
}

/******************************************************************************/
/*
	Send a close alert
*/
void sslWriteClosureAlert(sslConn_t *cp)
{
	unsigned char	*buf;
	int				len;
	
	if (cp != NULL) {
		if (matrixSslEncodeClosureAlert(cp->ssl) >= 0) {
			if ((len = matrixSslGetOutdata(cp->ssl, &buf)) > 0) {
				/* Non-blocking hail-mary alert */
				setSocketNonblock(cp->fd);
				if ((len = send(cp->fd, buf, len, MSG_DONTWAIT)) > 0) {
					matrixSslSentData(cp->ssl, len);
				}
			}
		}
	}
}

/******************************************************************************/
/*
	Close a seesion that was opened with sslAccept or sslConnect and
	free the insock and outsock buffers
*/
void sslFreeConnection(sslConn_t **cpp)
{
	sslConn_t	*conn;

	conn = *cpp;
	matrixSslDeleteSession(conn->ssl);
	conn->ssl = NULL;
	if (conn->pt != NULL) {
		bfree(B_L, conn->pt);
	}
	bfree(B_L, conn);
	*cpp = NULL;
}

/******************************************************************************/
/*
	Wait up to 'msec' time for write room to be available for 'fd'
	returns 0 if a subsequent write will succeed, -1 if it will fail
 */
static int waitForWriteEvent(int fd, int msec)
{
	struct timeval	tv;
	fd_set		 	writeFds;
	
	FD_ZERO(&writeFds);
	FD_SET(fd, &writeFds);
	tv.tv_sec = msec / 1000;
	tv.tv_usec = (msec % 1000) * 1000;
	if (select(fd + 1, NULL, &writeFds, NULL, &tv) > 0 && 
			FD_ISSET(fd, &writeFds)) {
		return 0;
	}
	return -1;
}

/******************************************************************************/
/*
	Turn off socket blocking mode.
 */
static void setSocketNonblock(SOCKET sock)
{
#if _WIN32
	int		block = 1;
	ioctlsocket(sock, FIONBIO, &block);
#elif LINUX
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
#endif
#ifdef MACOSX
	/* Prevent SIGPIPE when writing to closed socket on OS X */
	int		onoff = 1;
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&onoff, sizeof(onoff));
#endif
}


#endif /* WEBS_SSL_SUPPORT */

/******************************************************************************/
