/*
 * main.c -- Main program for the GoAhead WebServer (VxWorks version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include	<envLib.h>
#include	<hostLib.h>
#include	<iosLib.h>
#include	<loadLib.h>
#include	<sigLib.h>
#include	<sysSymTbl.h>
#include	<unldLib.h>

#include	"../uemf.h"
#include	"../wsIntrn.h"

#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void	formDefineUserMgmt(void);
#endif

/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */
#define				ROOT_DIR		T("/ata0/webs")

static char_t		*rootWeb = T("www");			/* Root web directory */
static char_t		*demoWeb = T("wwwdemo");		/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = 80;						/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished;						/* Finished flag */

/****************************** Forward Declarations **************************/

static int 	initWebs(int demo);
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
static void websTermSigHandler(int signo);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

/*********************************** Code *************************************/
/*
 *	Main -- entry point from VXWORKS
 */

int websvxmain(int argc, char **argv)
{
	int i, demo = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-demo") == 0) {
			demo++;
		}
	}

/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);

/*
 *	Initialize the web server
 */
	finished = 0;
	if (initWebs(demo) < 0) {
		return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block for two seconds or until an event
 *	occurs. SocketProcess will actually do the servicing.
 */
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 2000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
	symSubClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs(int demo)
{
	struct in_addr	intaddr;
	char			*pString;
	char			host[64], webdir[128];
	char_t			wbuf[128];

/*
 *	Initialize the socket and sym subsystems
 */
	socketOpen();
	symSubOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the 
 *	root web directory.
 */
	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}
	intaddr.s_addr = (unsigned long) hostGetByName(host);


/*
 *	Set ../web as the root web. Modify this to suit your needs
 */
	if (demo) {
		sprintf(webdir, "%s/%s", ROOT_DIR, demoWeb);
	} else {
		sprintf(webdir, "%s/%s", ROOT_DIR, rootWeb);
	}

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
	pString = inet_ntoa(intaddr);
	ascToUni(wbuf, pString, min(strlen(pString) + 1, sizeof(wbuf)));
	free(pString);
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
		WEBS_HANDLER_LAST); 

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 

/*
 *	Provide signal for clean up on termination.
 */
	signal(SIGTERM,	websTermSigHandler);
	signal(SIGKILL,	websTermSigHandler);

	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}
/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, T("home.asp"));
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Signal handler.  Process the terminate signals SIGTERM and SIGKILL.
 *	If the signal is SIGTERM, just set the flag so the next time
 *	through the event loop, the webserver will terminate itself cleanly.
 *	If the signal is SIGKILL, release the resources and exit immediately.
 */

static void websTermSigHandler(int signo)
{
	if (signo == SIGTERM) {
		finished = 1;
	} else if (signo == SIGKILL) {
#ifdef WEBS_SSL_SUPPORT
		websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
		umClose();
#endif
		websCloseServer();
		socketClose();
		symSubClose();
#ifdef B_STATS
		memLeaks();
#endif
		bclose();
		exit(1);
	}
}

/******************************************************************************/
/*
 *	Get absolute path.  In VxWorks, functions like chdir, ioctl for mkdir
 *	and ioctl for rmdir, require an absolute path.  This function will
 *	take the path argument and convert it to an absolute path.  It is the
 *	caller's responsibility to deallocate the returned string. 
 */

static char_t *getAbsolutePath(char_t *path)
{
	char_t	*tail;
	char_t	*dev;

/*
 *	Determine if path is relative or absolute.  If relative, prepend
 *	the current working directory to the name.  Otherwise, use it.
 *	Note the getcwd call below must not be ggetcwd or else we go into
 *	an infinite loop
 */
	if (iosDevFind(path, &tail) != NULL && path != tail) {
		return bstrdup(B_L, path);
	}
	dev = balloc(B_L, LF_PATHSIZE);
	getcwd(dev, LF_PATHSIZE);
	strcat(dev, "/");
	strcat(dev, path);
	return dev;
}

/******************************************************************************/
/*
 *	Change default working directory.
 */

int vxchdir(char_t *dirname)
{
	int		rc;
	char_t	*path;

/*
 *	Get an absolute path name for the directory.
 */
	path = getAbsolutePath(dirname);

/*
 *	Now change the default working directory.  The chdir call
 *	below must not be replaced with gchdir or else an infinite
 *	loop will occur.
 */
	rc = chdir(path);
	bfree(B_L, path);
	return rc;
}

/******************************************************************************/
#ifdef B_STATS
static void memLeaks() 
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0644)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/
