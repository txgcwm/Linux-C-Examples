/* 
 *	websSSL.h -- MatrixSSL Layer Header

 *
 *	(C)Copyright 2002-2010 PeerSec Networks
 *	All Rights Reserved
 */
/******************************************************************************/

#ifndef _h_websSSL
#define _h_websSSL 1

/******************************** Description *********************************/
/* 
 *	Header file for the PeerSec MatrixSSL layer. This defines the interface to 
 *	integrate MatrixSSL into the GoAhead Webserver.
 */

/********************************* Includes ***********************************/

#ifdef WEBS_SSL_SUPPORT

#include "webs.h"
#include "matrixSSLSocket.h"
#include "uemf.h"

/********************************** Defines ***********************************/

#define DEFAULT_CERT_FILE   "./certSrv.pem"		/* Public key certificate */
#define DEFAULT_KEY_FILE    "./privkeySrv.pem"	/* Private key file */

typedef struct {
	sslConn_t* sslConn;
	struct websRec* wp;
} websSSL_t;

/*************************** User Code Prototypes *****************************/

extern int	websSSLOpen();
extern int	websSSLIsOpen();
extern void websSSLClose();
#ifdef WEBS_WHITELIST_SUPPORT
extern int	websRequireSSL(char *url);
#endif /* WEBS_WHITELIST_SUPPORT */

/*************************** Internal Prototypes *****************************/

extern int	websSSLWrite(websSSL_t *wsp, char_t *buf, int nChars);
extern int	websSSLGets(websSSL_t *wsp, char_t **buf);
extern int	websSSLRead(websSSL_t *wsp, char_t *buf, int nChars);
extern int	websSSLEof(websSSL_t *wsp);

extern int	websSSLFree(websSSL_t *wsp);
extern int	websSSLFlush(websSSL_t *wsp);

extern int	websSSLSetKeyFile(char_t *keyFile);
extern int	websSSLSetCertFile(char_t *certFile);

#endif /* WEBS_SSL_SUPPORT */

#endif /* _h_websSSL */

/*****************************************************************************/
