/*
 * websda.c -- Digest Access Authentication routines
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Routines for generating DAA data.
 */

/********************************* Includes ***********************************/

#ifndef CE
#include	<time.h>
#endif
#include	"websda.h"
#include	"md5.h"

#ifdef DIGEST_ACCESS_SUPPORT
/******************************** Local Data **********************************/

#define RANDOMKEY	T("onceuponatimeinparadise")
#define NONCE_SIZE	34
#define HASH_SIZE   16

/*********************************** Code *************************************/
/*
 *	websMD5binary returns the MD5 hash
 */

char *websMD5binary(unsigned char *buf, int length)
{
    const char		*hex = "0123456789abcdef";
    psDigestContext_t	md5ctx;
    unsigned char	hash[HASH_SIZE];
    char			*r, *strReturn;
	char			result[(HASH_SIZE * 2) + 1];
    int				i;

/*
 *	Take the MD5 hash of the string argument.
 */
	psMd5Init(&md5ctx);
	psMd5Update(&md5ctx, buf, (unsigned int)length);
	psMd5Final(&md5ctx, hash);

/*
 *	Prepare the resulting hash string
 */
    for (i = 0, r = result; i < 16; i++) {
		*r++ = hex[hash[i] >> 4];
		*r++ = hex[hash[i] & 0xF];
    }

/*
 *	Zero terminate the hash string
 */
    *r = '\0';

/*
 *	Allocate a new copy of the hash string
 */
	i = elementsof(result);
	strReturn = balloc(B_L, i);
	strncpy(strReturn, result, i);

    return strReturn;
}

/*****************************************************************************/
/*
 *	Convenience call to websMD5binary 
 *	(Performs char_t to char conversion and back)
 */

char_t *websMD5(char_t *string)
{
	char_t	*strReturn;

	a_assert(string && *string);

	if (string && *string) {
		char	*strTemp, *strHash;
		int		nLen;
/*
 *		Convert input char_t string to char string
 */
		nLen = gstrlen(string);
		strTemp = ballocUniToAsc(string, nLen + 1);
/*
 *		Execute the digest calculation
 */
		strHash = websMD5binary((unsigned char *)strTemp, nLen);
/*
 *		Convert the returned char string digest to a char_t string
 */
		nLen = strlen(strHash);
		strReturn = ballocAscToUni(strHash, nLen);
/*
 *		Free up the temporary allocated resources
 */
		bfree(B_L, strTemp);
		bfree(B_L, strHash);
	} else {
		strReturn = NULL;
	}

	return strReturn;
}

/******************************************************************************/
/*
 *	Get a Nonce value for passing along to the client.  This function 
 *	composes the string "RANDOMKEY:timestamp:myrealm" and 
 *	calculates the MD5 digest placing it in output. 
 */

char_t *websCalcNonce(webs_t wp)
{
	char_t		*nonce, *prenonce;
	time_t		longTime;
#if defined(WIN32)
	char_t		buf[26];
	errno_t	error;
	struct tm	newtime;
#else
	struct tm	*newtime;
#endif

	a_assert(wp);
/*
 *	Get time as long integer.
 */
	time(&longTime);
/*
 *	Convert to local time.
 */
#if !defined(WIN32)
	newtime = localtime(&longTime);
#else
	
	error = localtime_s(&newtime, &longTime);
#endif
/*
 *	Create prenonce string.
 */
#if !defined(WIN32)
	prenonce = NULL;
	fmtAlloc(&prenonce, 256, T("%s:%s:%s"), RANDOMKEY, gasctime(newtime),
		wp->realm); 
#else
	asctime_s(buf, elementsof(buf), &newtime);
	fmtAlloc(&prenonce, 256, T("%s:%s:%s"), RANDOMKEY, buf, 
		RANDOMKEY); 
#endif

	a_assert(prenonce);
/*
 *	Create the nonce
 */
    nonce = websMD5(prenonce);
/*
 *	Cleanup
 */
	bfreeSafe(B_L, prenonce);

	return nonce;
}

/******************************************************************************/
/*
 *	Get an Opaque value for passing along to the client
 */

char_t *websCalcOpaque(webs_t wp)
{
	char_t *opaque;
	a_assert(wp);
/*
 *	Temporary stub!
 */
    opaque = bstrdup(B_L, T("5ccc069c403ebaf9f0171e9517f40e41"));

	return opaque;
}

/******************************************************************************/
/*
 *	Get a Digest value using the MD5 algorithm
 */

char_t *websCalcDigest(webs_t wp)
{
	char_t	*digest, *a1, *a1prime, *a2, *a2prime, *preDigest, *method;

	a_assert(wp);
	digest = NULL;

/*
 *	Calculate first portion of digest H(A1)
 */
	a1 = NULL;
	fmtAlloc(&a1, 255, T("%s:%s:%s"), wp->userName, wp->realm, wp->password);
	a_assert(a1);
	a1prime = websMD5(a1);
	bfreeSafe(B_L, a1);
/*
 *	Calculate second portion of digest H(A2)
 */
	method = websGetVar(wp, T("REQUEST_METHOD"), NULL);
	a_assert(method);
	a2 = NULL;
	fmtAlloc(&a2, 255, T("%s:%s"), method, wp->uri); 
	a_assert(a2);
	a2prime = websMD5(a2);
	bfreeSafe(B_L, a2);
/*
 *	Construct final digest KD(H(A1):nonce:H(A2))
 */
	a_assert(a1prime);
	a_assert(a2prime);
	a_assert(wp->nonce);

	preDigest = NULL;
	if (!wp->qop) {
		fmtAlloc(&preDigest, 255, T("%s:%s:%s"), a1prime, wp->nonce, a2prime);
	} else {
		fmtAlloc(&preDigest, 255, T("%s:%s:%s:%s:%s:%s"), 
			a1prime, 
			wp->nonce,
			wp->nc,
			wp->cnonce,
			wp->qop,
			a2prime);
	}

	a_assert(preDigest);
	digest = websMD5(preDigest);
/*
 *	Now clean up
 */
	bfreeSafe(B_L, a1prime);
	bfreeSafe(B_L, a2prime);
	bfreeSafe(B_L, preDigest);
	return digest;
}

/******************************************************************************/
/*
 *	Get a Digest value using the MD5 algorithm
 *	Digest is based on the full URL rather than the URI as above
 *	This alternate way of calculating is what most browsers use
 */

char_t *websCalcUrlDigest(webs_t wp)
{
	char_t	*digest, *a1, *a1prime, *a2, *a2prime, *preDigest, *method;

	a_assert(wp);
	digest = NULL;

/*
 *	Calculate first portion of digest H(A1)
 */
	a1 = NULL;
	fmtAlloc(&a1, 255, T("%s:%s:%s"), wp->userName, wp->realm, wp->password);
	a_assert(a1);
	a1prime = websMD5(a1);
	bfreeSafe(B_L, a1);
/*
 *	Calculate second portion of digest H(A2)
 */
	method = websGetVar(wp, T("REQUEST_METHOD"), NULL);
	a_assert(method);
	/* Fixes by Richard Laing, 2003/7/15 */
	a2 = balloc(B_L, (gstrlen(method) +2 + gstrlen(wp->url) ) * sizeof(char_t));
	a_assert(a2);
	gsprintf(a2, T("%s:%s"), method, wp->url);
	a2prime = websMD5(a2);
	bfreeSafe(B_L, a2);
/*
 *	Construct final digest KD(H(A1):nonce:H(A2))
 */
	a_assert(a1prime);
	a_assert(a2prime);
	a_assert(wp->nonce);

	preDigest = NULL;
	if (!wp->qop) {
		fmtAlloc(&preDigest, 255, T("%s:%s:%s"), a1prime, wp->nonce, a2prime);
	} else {
		fmtAlloc(&preDigest, 255, T("%s:%s:%s:%s:%s:%s"), 
			a1prime, 
			wp->nonce,
			wp->nc,
			wp->cnonce,
			wp->qop,
			a2prime);
	}

	a_assert(preDigest);
	digest = websMD5(preDigest);
/*
 *	Now clean up
 */
	bfreeSafe(B_L, a1prime);
	bfreeSafe(B_L, a2prime);
	bfreeSafe(B_L, preDigest);
	return digest;
}

#endif /* DIGEST_ACCESS_SUPPORT */
/******************************************************************************/

