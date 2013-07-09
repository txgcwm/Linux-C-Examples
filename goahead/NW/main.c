/*
 * main.c -- Main program for the GoAhead WebServer (NetWareversion)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 * Main program for for the GoAhead WebServer. This is a demonstration
 * main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include "../uemf.h"
#include "../wsIntrn.h"
#include <signal.h>
#include <sys/types.h>

#ifdef WEBS_SSL_SUPPORT
#include "../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include "../um.h"
void  formDefineUserMgmt(void);
#endif


/*********************************** Locals ***********************************/
/*
 * Change configuration here
 */

static char_t     *rootWeb = T("www");       /* Root web directory */
static char_t     *demoWeb = T("wwwdemo");   /* Root web directory */
static char_t     *password = T("");         /* Security password */
static int        port = 80;                 /* Server port */
static int        retries = 5;               /* Server port retries */
static int        finished;                  /* Finished flag */

/*********************************** Defines **********************************/
/*
 * Debug defines for testing only (Should be removed for final compile)
 */

#ifdef debug
   #define P( x ) printf( "\rWEBS: %d\n\r", x )
   #define Ps( x, s ) printf( "\rWEBS: %d [%s]\n\r", x, s )
#else
   #define P( x ) 
   #define Ps( x, s ) 
#endif

NETDB_DEFINE_CONTEXT ;
NETINET_DEFINE_CONTEXT ;

/****************************** Forward Declarations **************************/

static void NLMcleanup( void ) ;
static int  initWebs(int demo);
static int  aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
            int arg, char_t *url, char_t *path, char_t *query);

#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

void SigTermSignalHandler( int sigtype );
void NLMcleanup( void );

/*********************************** Code *************************************/
/*
 * Main 
 */

int main(int argc, char** argv)
{
	int i, demo = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-demo") == 0) {
			demo++;
		}
	}

/*
 *	Hook the unload routine in
 */
   signal( SIGTERM, SigTermSignalHandler ) ;

/*
 * Initialize the memory allocator. Allow use of malloc and start 
 * with a 60K heap.  For each page request approx 8KB is allocated.
 * 60KB allows for several concurrent page requests.  If more space
 * is required, malloc will be used for the overflow.
 */
   bopen(NULL, (60 * 1024), B_USE_MALLOC);

   P( 1 ) ;

/*
 *	Switch to LONG name-space. If LONG is not loaded, WEBS will default
 *	to 8.3
 */
   SetCurrentNameSpace( 4 ) ;

/*
 * Initialize the web server
 */
   if (initWebs(demo) < 0) {
      return -1;
   }

#ifdef WEBS_SSL_SUPPORT
   websSSLOpen();
#endif

   P( 20 ) ;

/*
 * Basic event loop. SocketReady returns true when a socket is ready for
 * service. SocketSelect will block until an event occurs. SocketProcess
 * will actually do the servicing.
 */
   while (!finished) {
      if (socketReady(-1) || socketSelect(-1, 1000)) {
         socketProcess(-1);
         ThreadSwitch() ;
      }
      websCgiCleanup();
      emfSchedProcess();
   }

   NLMcleanup() ;

   return 0;
}

/******************************************************************************/
/*
 * Initialize the web server.
 */

static int initWebs(int demo)
{
   struct hostent *hp;
   struct in_addr intaddr;
   char   host[128], dir[128], webdir[128];
   char   *cp;
   char_t wbuf[128];
   char_t *ipaddr;

   P( 2 ) ;
/*
 * Initialize the socket subsystem
 */
   socketOpen();

   P( 3 ) ;

#ifdef USER_MANAGEMENT_SUPPORT
/*
 * Initialize the User Management database
 */
   umOpen();
   umRestore(T("umconfig.txt"));
#endif

/*
 * Define the local Ip address, host name, default home page and the 
 * root web directory.
 */
   if (gethostname(host, sizeof(host)) < 0) {
      error(E_L, E_LOG, T("Can't get hostname"));
      return -1;
   }
   
   P( 4 ) ;
   
   if ((hp = gethostbyname(host)) == NULL) {
      error(E_L, E_LOG, T("Can't get host address"));
      return -1;
   }

   P( 5 ) ;

   memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
      (size_t) hp->h_length);


/*
 * Set ../web as the root web. Modify this to suit your needs
 */
   getcwd(dir, sizeof(dir)); 
   Ps( 6, dir ) ;
   if ((cp = strrchr(dir, '/'))) 
      cp ++ ;
   else
      cp = dir ;

   Ps( 6, cp ) ;

	if (demo) {
	   sprintf(webdir, "%s/%s", cp, demoWeb);
	} else {
	   sprintf(webdir, "%s/%s", cp, rootWeb);
	}

   Ps( 6, webdir ) ;

/*
 * Configure the web server options before opening the web server
 */
   websSetDefaultDir(webdir);
   ipaddr = inet_ntoa(intaddr);
   ascToUni(wbuf, ipaddr, gstrlen(ipaddr) + 1);
   websSetIpaddr(wbuf);
   ascToUni(wbuf, host, gstrlen(host) + 1);
   websSetHost(wbuf);

   P( 7 ) ;

/*
 * Configure the web server options before opening the web server
 */
   websSetDefaultPage(T("default.asp"));
   websSetPassword(password);

   P( 8 ) ;

/* 
 * Open the web server on the given port. If that port is taken, try
 * the next sequential port for up to "retries" attempts.
 */
   websOpenServer(port, retries);

   P( 9 ) ;

/*
 *	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
   websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
      WEBS_HANDLER_FIRST);
   websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
   websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
   websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
      WEBS_HANDLER_LAST);

   P( 10 ) ;

/*
 * Now define two test procedures. Replace these with your application
 * relevant ASP script procedures and form functions.
 */
   websAspDefine(T("aspTest"), aspTest);
   websFormDefine(T("formTest"), formTest);

   P( 11 ) ;

/*
 * Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
   formDefineUserMgmt();
#endif

/*
 * Create a handler for the default home page
 */
   websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);

   P( 12 ) ;

   return 0;
}

/******************************************************************************/
/*
 * Cleanup routine (prevents duplicate code)
 */

void NLMcleanup( void )
{
   P( 23 ) ;

#ifdef WEBS_SSL_SUPPORT
   websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
   umClose();
#endif

/*
 * Close the socket module, report memory leaks and close the memory allocator
 */
   websCloseServer();
   
   P( 24 ) ;

   socketClose();
#ifdef B_STATS
   memLeaks();
#endif
   P( 25 ) ;

   bclose();

   P( 26 ) ;
}

/******************************************************************************/
/*
 *	Routine to handle the UNLOAD command gracefully
 *	Note:	sigtype is not used
 */

#pragma off(unreferenced)
void SigTermSignalHandler( int sigtype )
#pragma on(unreferenced)
{
   finished ++ ;
   NLMcleanup() ;
}

/******************************************************************************/
/*
 * Test Javascript binding for ASP. This will be invoked when "aspTest" is
 * embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 * "localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t   *name, *address;

   if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
      websError(wp, 400, T("Insufficient args\n"));
      return -1;
   }
   return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 * Test form for posted data (in-memory CGI). This will be called when the
 * form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
   char_t   *name, *address;

   name = websGetVar(wp, T("name"), T("Joe Smith")); 
   address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

   websHeader(wp);
   websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
   websFooter(wp);
   websDone(wp, 200);
}

/******************************************************************************/
/*
 * Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
   int arg, char_t *url, char_t *path, char_t *query)
{
/*
 * If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
   if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
      websRedirect(wp, T("home.asp"));
      return 1;
   }
   return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks() 
{
   int      fd;

   if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY)) >= 0) {
      bstats(fd, printMemStats);
      close(fd);
   }
}

/******************************************************************************/
/*
 * Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
   va_list     args;
   char_t      buf[256];

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);
   write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/
