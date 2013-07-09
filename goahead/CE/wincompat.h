/*
 *	wincompat.h -- GoAhead compatibility header for CE
 *
 *	Copyright (c) Go Ahead Software, Inc., 1995-2010
 *
 */

#ifndef _h_WINCOMPAT
#define _h_WINCOMPAT 1

/*
 * the following replaces stdio.h
 */
typedef void FILE;

#define		BUFSIZ		1024
#define		PATHSIZE	1024

/*
 *	Declarations for the CE compatability functions we are providing.
 */
void exit(int status);
int _wmkdir(const unsigned short *path);
unsigned short* _wgetcwd(unsigned short *dir, int len);

extern long lseek(int handle, long offset, int origin);
#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

#define EOF     (-1)

/*
 * the following replaces fcntl.h
 */
#define O_RDONLY	0x0
#define O_WRONLY	0x1
#define O_RDWR		0x2
#define O_ACCMODE	0x3		/* Mask for O_RDONLY | O_WRONLY | O_RDWR */
#define O_NDELAY	0x4		/* Non-blocking I/O */
#define O_APPEND	0x8		/* append (writes guaranteed at the end) */
#define O_NONBLOCK	0x10	/* Non-blocking I/0, but different effect */
#define O_BINARY	0x20
#define O_CREAT		0x40
#define O_TRUNC		0x80
#define O_NOCTTY	0x100

extern int _wopen(const unsigned short* filename, int oflag, ...);
extern int read(int handle, void* buffer, unsigned int count);
extern int write(int handle, const void* buffer, unsigned int count);
extern int close(int handle);
extern int _wrename(const unsigned short* oldname, const unsigned short* newname);
extern FILE* fdopen(int handle, const unsigned short* mode);

/*
 * The following replaces getopt.h
 */
extern int	getopt(int, unsigned short * const *, const unsigned short *);

/*
 * the following replaces io.h and types.h
 */
#undef	u_long
#undef	u_short

typedef unsigned short mode_t;
typedef unsigned char u_char;
typedef unsigned int uint;
typedef unsigned int uint_t;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned long ulong;
typedef unsigned short u_short;
typedef unsigned short ushort;
typedef unsigned char unchar;
typedef char* caddr_t;
typedef short pid_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;

/*
 * the following replaces stdlib.h
 */

int _wunlink(unsigned short* filename);
int _wrename(const unsigned short* oldname, const unsigned short* newname);
int _waccess(const unsigned short* path, int mode);
int _wchdir(unsigned short* path);
int _wexecvp(unsigned short *path, unsigned short** argv);
int umask(int mode);
#if 0
/* 10 Apr 03 BgP -- CE.NET work. The following functions are now directly supported by CE, 
   so we do not need these compatibility versions.
*/
int	isupper(int c);
int	islower(int c);
int	isdigit(int c);
int	isspace(int c);
int	isprint(int c);
int isxdigit(int c);
#endif
/*
 * the following replaces stat.h
 */
struct _stat {
#if 0
	unsigned long st_size;	/* file size in bytes				*/
	unsigned long st_mode;
#else
   /* 11 Apr 03 BgP -- fixing an unsigned/signed comparison error */
	long st_size;	/* file size in bytes				*/
	long st_mode;
#endif
	time_t st_atime;		/* time of last access				*/
	time_t st_mtime;		/* time of last data modification	*/
	time_t st_ctime;		/* time of last file status change	*/
};

#define         S_IFREG 0100000
#define         S_IFDIR 0040000

extern int _wstat(unsigned short* path, struct _stat* buffer);
extern int fstat(int filenumber, struct _stat* buffer);

/*
 *	The following replaces errno.h
 */

#define EINTR		4
#define ENXIO		6
#define EBADF		9
#define EAGAIN		11
#define EINVAL		22

extern int errno;

/*
 * the following replaces time.h
 */
struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
#define LTZNMAX 50
	long tm_tzadj;
	char tm_name[LTZNMAX];	/* name of timezone	*/
};


extern unsigned short* _wctime(const time_t* timer);
extern time_t time(time_t* timer);
extern struct tm *localtime(const time_t* timer);
extern unsigned short *_wasctime(const struct tm *timeptr);

/*
 * the following replaces timeb.h
 */
struct timeb {

	time_t time;			/* time, seconds since the epoch */
	unsigned short millitm;	/* 1000 msec of additional accuracy */
	short timezone;			/* timezone, minutes west of GMT */
	short dstflag;			/* daylight savings */
};

extern void ftime(struct timeb* tp);

/*
 * the following replaces unistd.h
 */
extern long nap(long);
extern unsigned int sleep(unsigned int secs);
extern uid_t geteuid(void);

#define R_OK	4
#define W_OK	2
#define X_OK	1
#define F_OK	0

extern char* strdup(const char *s);

#endif /* _h_WINCOMPAT */
