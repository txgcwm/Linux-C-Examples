/*
 * url.c -- Parse URLs
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	This module parses URLs into their components.
 */

/********************************* Includes ***********************************/

#include	"wsIntrn.h"

/********************************* Statics ************************************/
/*
 *	htmExt is declared in this way to avoid a Linux and Solaris segmentation
 *	fault when a constant string is passed to strlower which could change its
 *	argument.
 */

char_t	htmExt[] = T(".htm");


/*********************************** Code *************************************/
/*
 *	Return the mime type for the given URL given a URL.
 *	The caller supplies the buffer to hold the result.
 *	charCnt is the number of characters the buffer will hold, ascii or UNICODE.
 */

char_t *websUrlType(char_t *url, char_t *buf, int charCnt)
{
	sym_t	*sp;
	char_t	*ext, *parsebuf;

	a_assert(url && *url);
	a_assert(buf && charCnt > 0);

	if (url == NULL || *url == '\0') {
		gstrcpy(buf, T("text/plain"));
		return buf;
	}
	if (websUrlParse(url, &parsebuf, NULL, NULL, NULL, NULL, NULL, 
			NULL, &ext) < 0) {
		gstrcpy(buf, T("text/plain"));
		return buf;
	}
	strlower(ext);

/*
 *	Lookup the mime type symbol table to find the relevant content type
 */
	if ((sp = symLookup(websMime, ext)) != NULL) {
		gstrncpy(buf, sp->content.value.string, charCnt);
	} else {
		gstrcpy(buf, T("text/plain"));
	}
	bfree(B_L, parsebuf);
	return buf;
}

/******************************************************************************/
/*
 *	Parse the URL. A buffer is allocated to store the parsed URL in *pbuf.
 *	This must be freed by the caller. NOTE: tag is not yet fully supported.
 */

int websUrlParse(char_t *url, char_t **pbuf, char_t **phost, char_t **ppath, 
	char_t **pport, char_t **pquery, char_t **pproto, char_t **ptag, 
	char_t **pext)
{
	char_t		*tok, *cp, *host, *path, *port, *proto, *tag, *query, *ext;
	char_t		*hostbuf, *portbuf, *buf;
	int			c, len, ulen;

	a_assert(url);
	a_assert(pbuf);

	ulen = gstrlen(url);
/*
 *	We allocate enough to store separate hostname and port number fields.
 *	As there are 3 strings in the one buffer, we need room for 3 null chars.
 *	We allocate MAX_PORT_LEN char_t's for the port number.
 */
	len = ulen * 2 + MAX_PORT_LEN + 3;
	if ((buf = balloc(B_L, len * sizeof(char_t))) == NULL) {
		return -1;
	}
	portbuf = &buf[len - MAX_PORT_LEN - 1];
	hostbuf = &buf[ulen+1];
   /*
   Handle any URL encoding.
   Otherwise a URL ending in ".as%70", for example, causes trouble.
   */
 	websDecodeUrl(buf, url, ulen);

	url = buf;

/*
 *	Convert the current listen port to a string. We use this if the URL has
 *	no explicit port setting
 */
	stritoa(websGetPort(), portbuf, MAX_PORT_LEN);
	port = portbuf;
	path = T("/");
	proto = T("http");
	host = T("localhost");
	query = T("");
	ext = htmExt;
	tag = T("");

	if (gstrncmp(url, T("http://"), 7) == 0) {
		tok = &url[7];
		tok[-3] = '\0';
		proto = url;
		host = tok;
		for (cp = tok; *cp; cp++) {
			if (*cp == '/') {
				break;
			}
			if (*cp == ':') {
				*cp++ = '\0';
				port = cp;
				tok = cp;
			}
		}
		if ((cp = gstrchr(tok, '/')) != NULL) {
/*
 *			If a full URL is supplied, we need to copy the host and port 
 *			portions into static buffers.
 */
			c = *cp;
			*cp = '\0';
			gstrncpy(hostbuf, host, ulen);
			gstrncpy(portbuf, port, MAX_PORT_LEN);
			*cp = c;
			host = hostbuf;
			port = portbuf;
			path = cp;
			tok = cp;
		}

	} else {
		path = url;
		tok = url;
	}

/*
 *	Parse the query string
 */
	if ((cp = gstrchr(tok, '?')) != NULL) {
		*cp++ = '\0';
		query = cp;
		path = tok;
		tok = query;
	} 

/*
 *	Parse the fragment identifier
 */
	if ((cp = gstrchr(tok, '#')) != NULL) {
		*cp++ = '\0';
		if (*query == 0) {
			path = tok;
		}
	}

/*
 *	Only do the following if asked for the extension
 */
	if (pext) {
		/*
		Later the path will be cleaned up for trailing slashes and so on.
		To be ready, we need to clean up here, much as in websValidateUrl.
		Otherwise a URL ending in "asp/" or "asP" sends Ejscript source
		to the browser.
		*/
		if ((cp = gstrrchr(path, '.')) != NULL) {
			const char_t* garbage = T("/\\");
			int length = gstrcspn(cp, garbage);
			int garbageLength = gstrspn(cp + length, garbage);
			int ok = (length + garbageLength == (int) gstrlen(cp));

			if (ok) {
				cp[length] = '\0';
#ifdef WIN32
				strlower(cp);            
#endif
				ext = cp;
			}
		}
	}

/*
 *	Pass back the fields requested (if not NULL)
 */
	if (phost)
		*phost = host;
	if (ppath)
		*ppath = path;
	if (pport)
		*pport = port;
	if (pproto)
		*pproto = proto;
	if (pquery)
		*pquery = query;
	if (ptag)
		*ptag = tag;
	if (pext)
		*pext = ext;
	*pbuf = buf;
	return 0;
}

/******************************************************************************/

