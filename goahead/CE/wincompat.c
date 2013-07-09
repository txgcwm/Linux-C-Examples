/*
 *	wincompat.c -- UNIX compatibility module for windows CE.
 *
 *	Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 *	See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/
/*
 *	This module provides a basic functions which provide compatibility between
 *	CE and other operating systems.
 */

/********************************* Includes ***********************************/

#define		IN_GETOPT

#include	<windows.h>

#ifdef UEMF
	#include "../uemf.h"
#else /* not UEMF */
	#include	"basic/basicInternal.h"
	#include	<direct.h>
	#include	<fcntl.h>
	#include	<getopt.h>
	#include	<io.h>
	#include	<pwd.h>
	#include	<stat.h>
	#include	<stdio.h>
	#include	<string.h>
	#include	<time.h>
	#include	<timeb.h>
	#include	<types.h>
	#include	<unistd.h>
	#include	<ulimit.h>
#endif /* UEMF */

/************************************* Defines ********************************/

typedef struct {
	HANDLE	hand;
} fh_t;

/********************************** Global Data *******************************/

int								errno;
__declspec(dllexport) int		opterr = 1;
__declspec(dllexport) int		optind = 1;
__declspec(dllexport) int		optopt;
__declspec(dllexport) char_t*	optarg;

/************************************* Locals *********************************/

static char	optswi = '-';

static fh_t			**f;			/* Array of all open files */
static int			fMax;			/* Maximum size of f array */

/*
 *	tm structure to be returned by localtime(). This is not thread safe.
 */
static struct tm localtm;

/*
 *	String returned by _wctime(). This is not thread safe.
 */
static char_t localctime[32];

static char_t *months[] = {
	T("Jan"), T("Feb"), T("Mar"), T("Apr"), T("May"), T("Jun"), T("Jul"),
	T("Aug"), T("Sep"), T("Oct"), T("Nov"), T("Dec")
};

static char_t *days[] = {
	T("Sun"), T("Mon"), T("Tue"), T("Wed"), T("Thu"), T("Fri"), T("Sat")
};

static char_t ceCwd[LF_PATHSIZE];

char_t *_wgetcwd(char_t *dir, int len);
int _wchdir(unsigned short *path);

/************************************ Code ************************************/

uid_t geteuid(void)
{
	return 0;
}

/******************************************************************************/

int	getopt(int argc, char_t* const * argv, const char_t* opts)
{
	char_t		*cp;
	char_t		noswitch[3];
	static int	sp = 1;
	int			c;

	gstrnset(noswitch, optswi, 2);
	noswitch[2]=0;
	if (sp == 1) {
		if (optind >= argc ||
			argv[optind][0] != optswi || argv[optind][1] == '\0') {
			return(EOF);
		}
		else if (gstrcmp(argv[optind], noswitch) == 0) {
			optind++;
			return(EOF);
		}
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || (cp = gstrchr(opts, (char_t) c)) == NULL) {
		if (argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if (*++cp == ':') {
		if (argv[optind][sp+1] != '\0') {
			optarg = &argv[optind++][sp+1];
		} else if (++optind >= argc) {
			sp = 1;
			return('?');
		} else {
			optarg = argv[optind++];
		}
		sp = 1;
	} else {
		if (argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

/******************************************************************************/
/*
 * sleep -- Sleep for N seconds
 */

unsigned sleep(unsigned s)
{
	Sleep(s * 1000);
	return 0;
}

/******************************************************************************/
/*
 * nap -- Sleep for N milli-seconds
 */

long nap(long ms)
{
	Sleep(ms);
	return 0;
}

/******************************************************************************/
/*
 *	Return the elapsed time in milli-seconds
 */

time_t timeMsec()
{
	struct timeb	tbnow;
	time_t			sec;
	int				msec;
	static time_t	start = 0;
	static time_t	start_msec;

	ftime(&tbnow);
	if (start == 0) {
		start = tbnow.time;
		start_msec = tbnow.millitm;
	}
	sec = tbnow.time - start;
	msec = tbnow.millitm - start_msec;
	if (msec < 0) {
		msec += 1000;
		sec -= 1;
	}
	return sec * 1000 + msec;
}

/******************************************************************************/
/*
 *	We want the time in seconds since the start of the UNIX epoch (1970). We
 *	can get the number of 100-nanoseconds since 1601 (FILETIME). We simply
 *	convert this to seconds and subtract the number of seconds between 1601
 *	and 1970!
 */

#define	SECS_BETWEEN_1601_AND_1970	3054510208

static time_t fileTimeToUnixEpochTime(FILETIME f)
{
	ULARGE_INTEGER	uli;

	uli.LowPart = f.dwLowDateTime;
	uli.HighPart = f.dwHighDateTime;
	uli.QuadPart /= 10000000;

	return((time_t) uli.QuadPart - SECS_BETWEEN_1601_AND_1970);
}

/******************************************************************************/
/*
 *	Mock the UNIX time() call!
 */

time_t time(time_t *timer)
{
	SYSTEMTIME		stime;
	FILETIME		ftime;
	time_t			tmp;

	GetLocalTime(&stime);
	SystemTimeToFileTime(&stime, &ftime);
	tmp = fileTimeToUnixEpochTime(ftime);
	if (timer != NULL) {
		*timer = tmp;
	}
	return tmp;
}

/******************************************************************************/
/*
 *	Mock the UNIX localtime() call.
 */

struct tm* localtime(const time_t *timer)
{
	SYSTEMTIME		stime;
	GetLocalTime(&stime);
	
	localtm.tm_sec = stime.wSecond;
	localtm.tm_min = stime.wMinute;
	localtm.tm_hour = stime.wHour;
	localtm.tm_mday = stime.wDay;
	localtm.tm_wday = stime.wDayOfWeek;
	localtm.tm_mon = stime.wMonth - 1;
	localtm.tm_year = stime.wYear - 1900;
/*
 *	The following values are not supported yet, but the hardcoded assignments
 *	should suffice.
 */
	localtm.tm_yday = -1;
	localtm.tm_isdst = -1;
	localtm.tm_tzadj = -1;
	localtm.tm_name[0] = 0;
	return &localtm;
}

#ifndef LITTLEFOOT
/******************************************************************************/
/*
 *	Mock the UNIX ctime() call.
 */

char_t* _wctime(const time_t *timer)
{
	SYSTEMTIME		stime;
	FILETIME		ftime;
	ULARGE_INTEGER	uli;

	uli.QuadPart = *timer * 10000000;
	ftime.dwLowDateTime = uli.LowPart;
	ftime.dwHighDateTime = uli.HighPart;

	if (FileTimeToSystemTime(&ftime, &stime) == 0) {
		return NULL;
	}

	wsprintf(localctime, T("%s %s %02d %02d:%02d:%02d %04d\n"),
		days[stime.wDayOfWeek], months[stime.wMonth - 1], stime.wDay,
		stime.wHour, stime.wMinute, stime.wSecond, stime.wYear);
	return localctime;
}

/******************************************************************************/
/*
 *	Our implementation of _wasctime.
 */

char_t *_wasctime(const struct tm *timeptr)
{
	static char_t	strTime[52];

	wsprintf(strTime, T("%s %s %02d %02d:%02d:%02d %04d\n"),
		days[timeptr->tm_wday], months[timeptr->tm_mon], timeptr->tm_mday,
		timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec, 
		(timeptr->tm_year + 1900));

	return &strTime[0];
}
#endif	/* !LITTLEFOOT */

/******************************************************************************/
/*
 *	Mock the UNIX ftime() call.
 */

void ftime(struct timeb *tp)
{
	tp->time = time(NULL);
	tp->millitm = 0;
	return;
#ifdef UNSUPPORTED
	short timezone;			/* timezone, minutes west of GMT */
	short dstflag;			/* daylight savings */
#endif /* UNSUPPORTED */
}

/******************************************************************************/
/*
 *	Open a file.
 */

int _wopen(const char_t *path, int oflag, ...)
{
	HANDLE	hFile;
	DWORD	dwDesiredAccess, dwShareMode, dwCreationDisposition;
	fh_t	*fp;
	int		fid;
	char_t	*cwd, *filepath;

	filepath = balloc(B_L, LF_PATHSIZE * sizeof(char_t));
	cwd = balloc(B_L, LF_PATHSIZE * sizeof(char_t));

	if ((*path != '/') && (*path != '\\')) {
		cwd = _wgetcwd(cwd, LF_PATHSIZE);
		if (cwd == NULL) {
			return -1;
		}
		fmtAlloc(&filepath, LF_PATHSIZE, T("%s/%s"), cwd, path);
	} else {
		wcscpy(filepath, path);
	}
	
	dwDesiredAccess = 0;
	if (((oflag & 0xF) == O_RDONLY) || (oflag & O_RDWR)) {
		dwDesiredAccess |= GENERIC_READ;
	}
	if (oflag & O_WRONLY || oflag & O_RDWR) {
		dwDesiredAccess |= GENERIC_WRITE;
	}
	dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if (oflag & O_CREAT) {
		dwCreationDisposition = OPEN_ALWAYS;
	} else {
		dwCreationDisposition = OPEN_EXISTING;
	}

	if ((hFile = CreateFile(filepath, dwDesiredAccess, dwShareMode, NULL,
			dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 
			NULL)) == INVALID_HANDLE_VALUE) {
		bfree(B_L, filepath);
		bfree(B_L, cwd);
		return -1;
	}

/*
 *	Get a file number to associate with the handle.
 */
	if ((fid = hAllocEntry((void***) &f, &fMax, sizeof(fh_t))) < 0) {
		bfree(B_L, filepath);
		bfree(B_L, cwd);
		return -1;
	}
	fp = f[fid];
	fp->hand = hFile;
	bfree(B_L, filepath);
	bfree(B_L, cwd);

	return fid;
}

/******************************************************************************/
/*
 *	Close a file.
 */

int close(int fid)
{
	fh_t	*fp;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL) {
		return -1;
	}
	CloseHandle(fp->hand);
	fMax = hFree((void***) &f, fid);
	bfreeSafe(B_L, fp);

	return 0;
}

/******************************************************************************/
/*
 *	Read bytes from a file.
 */

int read(int fid, void *buf, unsigned int len)
{
	fh_t	*fp;
	long	bytesRead;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL) {
		return -1;
	}

	if (ReadFile(fp->hand, buf, len, &bytesRead, NULL) == 0) {
		return -1;
	}
	return bytesRead;
}

/******************************************************************************/
/*
 *	Read from a file into a buffer, converting from Ascii to Unicode.
 */

int readAscToUni(int fid, void **buf, unsigned int len)
{
	fh_t	*fp;
	long	bytesRead;
	char_t	*uniBuf;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL) {
		return -1;
	}

	if (ReadFile(fp->hand, *buf, len, &bytesRead, NULL) == 0) {
		return -1;
	}

	uniBuf =  ballocAscToUni(*buf, len);
	bfree(B_L, *buf);
	*buf = uniBuf;

	return bytesRead;
}

/******************************************************************************/
/*
 *	Write to a file.
 */

int write(int fid, void *buf, unsigned int len)
{
	fh_t	*fp;
	long	bytesWritten;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL) {
		return -1;
	}

	if (WriteFile(fp->hand, buf, len, &bytesWritten, NULL) == 0) {
		return -1;
	}
	return bytesWritten;
}

/******************************************************************************/
/*
 *	Write to a file, first converting from Unicode to Ascii.
 */

int writeUniToAsc(int fid, void *buf, unsigned int len)
{
	fh_t	*fp;
	long	bytesWritten;
	char	*asciiBuf;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL) {
		return -1;
	}

	asciiBuf =  ballocUniToAsc(buf, len);

	if (WriteFile(fp->hand, asciiBuf, len, &bytesWritten, NULL) == 0) {
		bfree(B_L, asciiBuf);
		return -1;
	}
	bfree(B_L, asciiBuf);
	return bytesWritten;
}

/******************************************************************************/
/*
 *	Delete a file.
 */

int _wunlink(char_t *file)
{
	char_t	*cwd, *cwdFile;

	cwdFile = NULL;
	if ((*file != '/') && (*file != '\\')) {
		cwd = balloc(B_L, LF_PATHSIZE);
		ggetcwd(cwd, LF_PATHSIZE / sizeof(char_t));
		fmtAlloc(&cwdFile, LF_PATHSIZE / sizeof(char_t), T("%s/%s"), cwd, file);
		bfree(B_L, cwd);
		file = cwdFile;
	}

	if (DeleteFile(file) != TRUE) {
		bfreeSafe(B_L, cwdFile);
		return -1;
	}
	bfreeSafe(B_L, cwdFile);
	return 0;
}

/******************************************************************************/
/*
 *	Create a new directory
 */

int _wmkdir(const char_t *path)
{
	char_t	*cwd, *cwdPath;

	cwdPath = NULL;
	if ((*path != '/') && (*path != '\\')) {
		cwd = balloc(B_L, LF_PATHSIZE);
		ggetcwd(cwd, LF_PATHSIZE / sizeof(char_t));
		fmtAlloc(&cwdPath, LF_PATHSIZE, T("%s/%s"), cwd, path);
		bfree(B_L, cwd);
		path = cwdPath;
	}

	if (CreateDirectory(path, NULL) == 0) {
		bfreeSafe(B_L, cwdPath);
		return -1;
	}
	bfreeSafe(B_L, cwdPath);
	return 0;
}

/******************************************************************************/
/*
 *	Remove a directory
 */

int _wrmdir(const char_t *path)
{
	char_t	*cwd, *cwdPath;

	cwdPath = NULL;
	if ((*path != '/') && (*path != '\\')) {
		cwd = balloc(B_L, LF_PATHSIZE);
		ggetcwd(cwd, LF_PATHSIZE / sizeof(char_t));
		fmtAlloc(&cwdPath, LF_PATHSIZE, T("%s/%s"), cwd, path);
		bfree(B_L, cwd);
		path = cwdPath;
	}

	if (RemoveDirectory(path) == 0) {
		bfreeSafe(B_L, cwdPath);
		return -1;
	}
	bfreeSafe(B_L, cwdPath);
	return 0;
}

/******************************************************************************/
/*
 *	Get file information about an open file.
 */

int fstat(int fid, struct _stat *sbuf)
{
	BY_HANDLE_FILE_INFORMATION	info;
	long						size;
	fh_t						*fp;

	a_assert(0 <= fid && fid < fMax);
	fp = f[fid];
	if (fp == NULL || fp->hand == INVALID_HANDLE_VALUE) {
		return -1;
	}

	size = GetFileSize(fp->hand, NULL);
	if (size == 0xFFFFFFFF) {
		return -1;
	}
	sbuf->st_size = (int) size;

	if (GetFileInformationByHandle(fp->hand, &info)) {
		if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			sbuf->st_mode = S_IFDIR;
		} else {
			sbuf->st_mode = S_IFREG;
		}
		sbuf->st_mtime = fileTimeToUnixEpochTime(info.ftLastWriteTime);
		return 0;
	}

	return -1;
}

/******************************************************************************/
/*
 *	Our implementation of wide character stat: get file information.
 */

int _wstat(char_t *filename, struct _stat *sbuf)
{
	DWORD	dwAttributes;
	int		fid, rc;

	memset(sbuf, 0, sizeof(struct _stat));
	dwAttributes = GetFileAttributes(filename);
	if (dwAttributes != 0xFFFFFFFF && dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		sbuf->st_mode = S_IFDIR;
		return 0;
	}
	sbuf->st_mode = S_IFREG;

	if ((fid = _wopen(filename, 0, 0)) < 0) {
		return -1;
	}

	rc = fstat(fid, sbuf);
	close(fid);

	return rc;		 
}

/******************************************************************************/
/*
 *	Our implementation of wide character rename: change the name of a file.
 */

int _wrename(const char_t *oldname, const char_t *newname)
{
	char_t	*cwd, *cwdOldname, *cwdNewname;

	cwdOldname = cwd = NULL;
	if ((*oldname != '/') && (*oldname != '\\')) {
		cwd = balloc(B_L, LF_PATHSIZE);
		ggetcwd(cwd, LF_PATHSIZE / sizeof(char_t));
		fmtAlloc(&cwdOldname, LF_PATHSIZE, T("%s/%s"), cwd, oldname);
		oldname = cwdOldname;
	}
	
	cwdNewname = NULL;
	if ((*newname != '/') && (*newname != '\\')) {
		if (cwd != NULL) {
			cwd = balloc(B_L, LF_PATHSIZE);
			ggetcwd(cwd, LF_PATHSIZE / sizeof(char_t));
		}
		fmtAlloc(&cwdNewname, LF_PATHSIZE, T("%s/%s"), cwd, newname);
		newname = cwdNewname;
	}

	if (MoveFile(oldname, newname) == 0) {
		bfreeSafe(B_L, cwdOldname);
		bfreeSafe(B_L, cwdNewname);
		bfreeSafe(B_L, cwd);
		return 1;
	} else {
		bfreeSafe(B_L, cwdOldname);
		bfreeSafe(B_L, cwdNewname);
		bfreeSafe(B_L, cwd);
		return 0;
	}
}

/******************************************************************************/
/*
 *	Our implementation of wide character access: get file access permission.
 */

int _waccess(const unsigned short *path, int mode)
{
	int	fid, omode;

	if (mode == R_OK) {
		omode = O_RDONLY;
	} else if (mode == W_OK) {
		omode = O_RDWR;
	} else {
		a_assert(0);
	}
	if ((fid = _wopen(path, omode)) < 0) {
		return -1;
	}
	close(fid);
	return 0;
}

/******************************************************************************/
/*
 *	Support for stream I/O. The only way we supply a FILE* is via fdopen. In
 *	this case we return the real file handle.
 */

FILE* fdopen(int handle, const unsigned short *mode)
{
	return (FILE*) handle;
}

#if 0
int fclose(FILE *stream)
{
	fh_t	*fp;
	int		fid;

	for (fid = 0; fid > fMax; fid++) {
		if ((fp = f[fid]) != NULL) {
			if (fp->hand == stream) {
				close(fid);
				return 0;
			}
		}
	}
	return EOF;
}

int fflush(FILE* stream)
{
	return 0;
}
#endif /* 0 */

/******************************************************************************/
/*
 * lseek  -  position a file.
 */
long lseek(int handle, long offset, int origin)
{
	fh_t	*fp;

	fp = f[handle];
	return SetFilePointer(fp->hand, offset, NULL, origin);
}

/******************************************************************************/
/*
 *	A bunch of ASCII character routines.
 */

/******************************************************************************/
/*
 *	Our implementation of isupper.
 */
#if 0
/* 10 Apr 03 BgP -- CE.NET work. The following functions are now directly supported by CE, 
   so we do not need these compatibility versions.
*/
int isupper(int c)
{
	if (c >= 'A' && c <= 'Z') {
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Our implementation of islower.
 */

int islower(int c)
{
	if (c >= 'a' && c <= 'z') {
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Our implementation of isdigit.
 */

int isdigit(int c)
{
	if (c >= '0' && c <= '9') {
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Our implementation of isspace.
 */

int isspace(int c)
{
	if (c >= 0x9 && c <= 0xd || c == 0x20) {
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Our implementation of isprint.
 */

int isprint(int c)
{
	if (c >= 0x20 && c <= 0x7e) {
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Our implementation of isxdigit.
 */

int isxdigit(int c)
{
	if ((c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F')) {
		return 1;
	}
	return 0;
}

#endif

/******************************************************************************/
#ifndef LITTLEFOOT
int umask(int mode)
{
	return 0;
}
#endif

/******************************************************************************/
/*
 * If the application needs wprintf functionality, insert it here.
 */
#if 0
int wprintf(const char_t *fmt, ...)
{
	return 0;
}
#endif

#if 0
int fwprintf(FILE *stream, const char_t *fmt, ...)
{
	return 0;
}

int fputws(const char_t *fmt, FILE *stream)
{
	return 0;
}
#endif /* 0 */

#ifndef LITTLEFOOT
int _wexecvp(char_t *path, char_t** argv)
{
	return -1;
}
#endif

/******************************************************************************/
/*
 *	Return the current working directory
 */

char_t *_wgetcwd(char_t *dir, int len)
{
	if (len > LF_PATHSIZE) {
		return NULL;
	} else {
		return wcscpy(dir, ceCwd);
	}
}

/******************************************************************************/
/*
 *	Change the current working directory
 */

int _wchdir(unsigned short *path)
{
	char_t	*p;
	char_t	buf[LF_PATHSIZE];
	int		len;
	gstat_t	statDir;

/*
 *	Ignore leading spaces
 */
	while(*path == ' ') {
		path++;
	}

	if (path == NULL || *path == '\0') {
		return -1;
	} else if ((*path == '/') || (*path == '\\')) {
		 wcscpy(ceCwd, path);
	} else {
/*
 *		Otherwise, it's a relative path.  Combine with current path.
 *		Append a '/' if the last character is not already a '/'.
 *		Check for leading "./" or "../" and back up the cwd accordingly.
 */

		for (p = path; *p; p++) {
			if (*p == '\\') {
				*p = '/';
			}
		}

		gstrcpy(buf, ceCwd);
		for (p = buf; *p; p++) {
			if (*p == '\\') {
				*p = '/';
			}
		}

		if (*path == '.' && *(path+1) == '/') {
			path++;
			while (*path == '/') {
				path++;
			}
		}
		while (*path == '.' && *(path+1) == '.') {
			path += 2;
			if ((p = wcsrchr(buf, '/')) == NULL) {
				return -1;
			}
			*p = '\0';
			if (*path == '\0') {
				break;
			}
			if (*path != '/') {
				return -1;
			}
			while (*path == '/') {
				path++;
			}
		}
/*
 *		Ignore leading spaces and dots
 */
		while(*path == '.' || *path == ' ') {
			path++;
		}

/*
 *		Append to cwd.
 */
		len = wcslen(buf);
		if (*(buf + len - 1) != '/') {
			*(buf + len) = '/';
			*(buf + len + 1) = '\0';
			len++;
		}
		if ((len + gstrlen(path) + 1) > LF_PATHSIZE ) {
			return -1;
		}
		gstrcpy(buf+len, path);

		if (_wstat(buf, &statDir) == -1) {
			return -1;
		}
		gstrcpy(ceCwd, buf);
	}
	return 0;
}

/******************************************************************************/
/*
 *	Change the file-permission settings
 */

int _wchmod(const char_t *filename, int pmode)
{
	return 0;
}

/******************************************************************************/
