/*
 *	md5.h
 *	Release $Name: WEBSERVER-2-5 $
 *
 *	(C)Copyright 2002-2010 PeerSec Networks
 *	All Rights Reserved
 *
 */
/******************************************************************************/

#ifndef _h_MD5
#define _h_MD5 1

#ifndef WEBS_SSL_SUPPORT

#ifndef ulong32
typedef unsigned int	ulong32;
#endif

typedef struct {
    ulong32 lengthHi;
    ulong32 lengthLo;
    ulong32 state[4], curlen;
    unsigned char buf[64];
} psDigestContext_t;

typedef psDigestContext_t	psMd5Context_t;

extern void psMd5Init(psMd5Context_t *md);
extern void psMd5Update(psMd5Context_t *md, unsigned char *buf,
			unsigned int len);
extern int	psMd5Final(psMd5Context_t *md, unsigned char *hash);

/* Uncomment below for old API Compatibility */
/*
typedef psMdContext_t		MD5_CONTEXT;
#define MD5Init(A)			psMd5Init(A)
#define MD5Update(A, B, C)	psMd5Update(A, B, C);
#define MD5Final(A, B)		psMd5Final(B, A);
*/

#endif /* WEBS_SSL_SUPPORT */
#endif /* _h_MD5 */
