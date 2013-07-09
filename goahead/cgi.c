/*
 * cgi.c -- CGI processing (for the GoAhead Web server
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/********************************** Description *******************************/
/*
 *	This module implements the /cgi-bin handler. CGI processing differs from
 *	goforms processing in that each CGI request is executed as a separate 
 *	process, rather than within the webserver process. For each CGI request the
 *	environment of the new process must be set to include all the CGI variables
 *	and its standard input and output must be directed to the socket.  This
 *	is done using temporary files.
 */

/*********************************** Includes *********************************/
#include	"wsIntrn.h"
#include	"uemf.h"


/************************************ Locals **********************************/
typedef struct {				/* Struct for CGI tasks which have completed */
	webs_t	wp;					/* pointer to session websRec */
	char_t	*stdIn;				/* file desc. for task's temp input fd */
	char_t	*stdOut;			/* file desc. for task's temp output fd */
	char_t	*cgiPath;			/* path to executable process file */
	char_t	**argp;				/* pointer to buf containing argv tokens */
	char_t	**envp;				/* pointer to array of environment strings */
	int		handle;				/* process handle of the task */
	long	fplacemark;			/* seek location for CGI output file */
} cgiRec;
static cgiRec	**cgiList;		/* hAlloc chain list of wp's to be closed */
static int		cgiMax;			/* Size of hAlloc list */

/************************************* Code ***********************************/

/*
 *	Process a form request. Returns 1 always to indicate it handled the URL
 */
int websCgiHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		char_t *url, char_t *path, char_t* query)
{
	cgiRec		*cgip;
	sym_t		*s;
	char_t		cgiBuf[FNAMESIZE], *stdIn, *stdOut, cwd[FNAMESIZE];
	char_t		*cp, *cgiName, *cgiPath, **argp, **envp, **ep;
	int			n, envpsize, argpsize, pHandle, cid, rc;
	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path == '/');
/*
 *  If the whitelist check fails, try to rebuild the list right away 
 *  and try one more time. Whitelist just checks that the file exists
 *	under the current www root.  We will check if it is executable below.
 */
#ifdef WEBS_WHITELIST_SUPPORT
	if ((rc = websWhitelistCheck(wp->url)) < 0 || !(rc & WHITELIST_CGI)) {
		websBuildWhitelist();
		if ((rc = websWhitelistCheck(wp->url)) < 0 || !(rc & WHITELIST_CGI)) {
			websError(wp, 500, T("Invalid CGI URL"));
			return 1;
		}
	}
	if (!(wp->flags & WEBS_SECURE) && (rc & WHITELIST_SSL)) {
		websError(wp, 500, T("HTTPS Access Required"));
		return 1;
	}
#endif /* WEBS_WHITELIST_SUPPORT */

	websStats.cgiHits++;

/*
 *	Extract the form name and then build the full path name.  The form
 *	name will follow the first '/' in path.
 */
	gstrncpy(cgiBuf, path, TSZ(cgiBuf));
	if ((cgiName = gstrchr(&cgiBuf[1], '/')) == NULL) {
		websError(wp, 200, T("Missing CGI name"));
		return 1;
	}
	cgiName++;
	if ((cp = gstrchr(cgiName, '/')) != NULL) {
		*cp = '\0';
	}
	fmtAlloc(&cgiPath, FNAMESIZE, T("%s/%s/%s"), websGetDefaultDir(),
		CGI_BIN, cgiName);
#ifndef VXWORKS
/*
 *	See if the file exists and is executable.  If not error out.
 *	Don't do this step for VxWorks, since the module may already
 *	be part of the OS image, rather than in the file system.
 */
	{
		gstat_t		sbuf;
		if (gstat(cgiPath, &sbuf) != 0 || (sbuf.st_mode & S_IFREG) == 0) {
			websError(wp, 404, T("CGI process file does not exist"));
			bfree(B_L, cgiPath);
			return 1;
		}
#if (defined (WIN) || defined (CE))
		if (gstrstr(cgiPath, T(".exe")) == NULL &&
			gstrstr(cgiPath, T(".bat")) == NULL) {
#elif (defined (NW))
			if (gstrstr(cgiPath, T(".nlm")) == NULL) {
#else
		if (gaccess(cgiPath, X_OK) != 0) {
#endif /* WIN || CE */
			websError(wp, 200, T("CGI process file is not executable"));
			bfree(B_L, cgiPath);
			return 1;
		}
	}
#endif /* ! VXWORKS */

         
/*
 *	Get the CWD for resetting after launching the child process CGI
 */
	ggetcwd(cwd, FNAMESIZE);
/*
 *	Retrieve the directory of the child process CGI
 */
	if ((cp = gstrrchr(cgiPath, '/')) != NULL) {
		*cp = '\0';
		gchdir(cgiPath);
		*cp = '/';
	}
/*
 *	Build command line arguments.  Only used if there is no non-encoded
 *	= character.  This is indicative of a ISINDEX query.  POST separators
 *	are & and others are +.  argp will point to a balloc'd array of 
 *	pointers.  Each pointer will point to substring within the
 *	query string.  This array of string pointers is how the spawn or 
 *	exec routines expect command line arguments to be passed.  Since 
 *	we don't know ahead of time how many individual items there are in
 *	the query string, the for loop includes logic to grow the array 
 *	size via brealloc.
 */
	argpsize = 10;
	argp = balloc(B_L, argpsize * sizeof(char_t *));
	*argp = cgiPath;
	n = 1;
	if (gstrchr(query, '=') == NULL) {
		websDecodeUrl(query, query, gstrlen(query));
		for (cp = gstrtok(query, T(" ")); cp != NULL; ) {
			*(argp+n) = cp;
			n++;
			if (n >= argpsize) {
				argpsize *= 2;
				argp = brealloc(B_L, argp, argpsize * sizeof(char_t *));
			}
			cp = gstrtok(NULL, T(" "));
		}
	}
	*(argp+n) = NULL;
/*
 *	Add all CGI variables to the environment strings to be passed
 *	to the spawned CGI process.  This includes a few we don't 
 *	already have in the symbol table, plus all those that are in
 *	the cgiVars symbol table.  envp will point to a balloc'd array of 
 *	pointers.  Each pointer will point to a balloc'd string containing
 *	the keyword value pair in the form keyword=value.  Since we don't
 *	know ahead of time how many environment strings there will be the
 *	for loop includes logic to grow the array size via brealloc.
 */
	envpsize = WEBS_SYM_INIT;
	envp = balloc(B_L, envpsize * sizeof(char_t *));
	n = 0;
	fmtAlloc(envp+n, FNAMESIZE, T("%s=%s"),T("PATH_TRANSLATED"), cgiPath);
	n++;
	fmtAlloc(envp+n, FNAMESIZE, T("%s=%s/%s"),T("SCRIPT_NAME"),
		CGI_BIN, cgiName);
	n++;
	fmtAlloc(envp+n, FNAMESIZE, T("%s=%s"),T("REMOTE_USER"), wp->userName);
	n++;
	fmtAlloc(envp+n, FNAMESIZE, T("%s=%s"),T("AUTH_TYPE"), wp->authType);
	n++;
	for (s = symFirst(wp->cgiVars); s != NULL; s = symNext(wp->cgiVars)) {
		if (s->content.valid && s->content.type == string &&
			gstrcmp(s->name.value.string, T("REMOTE_HOST")) != 0 &&
			gstrcmp(s->name.value.string, T("HTTP_AUTHORIZATION")) != 0) {
			fmtAlloc(envp+n, FNAMESIZE, T("%s=%s"), s->name.value.string,
				s->content.value.string);
			n++;
			if (n >= envpsize) {
				envpsize *= 2;
				envp = brealloc(B_L, envp, envpsize * sizeof(char_t *));
			}
		}
	}
	*(envp+n) = NULL;
/*
 *	Create temporary file name(s) for the child's stdin and stdout.
 *	For POST data the stdin temp file (and name) should already exist.
 */
	if (wp->cgiStdin == NULL) {
		wp->cgiStdin = websGetCgiCommName();
	} 
	stdIn = wp->cgiStdin;
	stdOut = websGetCgiCommName();
/*
 *	Now launch the process.  If not successful, do the cleanup of resources.
 *	If successful, the cleanup will be done after the process completes.
 */
	if ((pHandle = websLaunchCgiProc(cgiPath, argp, envp, stdIn, stdOut)) 
		== -1) {
		websError(wp, 200, T("failed to spawn CGI task"));
		for (ep = envp; *ep != NULL; ep++) {
			bfreeSafe(B_L, *ep);
		}
		bfreeSafe(B_L, cgiPath);
		bfreeSafe(B_L, argp);
		bfreeSafe(B_L, envp);
		bfreeSafe(B_L, stdOut);
	} else {
/*
 *		If the spawn was successful, put this wp on a queue to be
 *		checked for completion.
 */
		cid = hAllocEntry((void***) &cgiList, &cgiMax, sizeof(cgiRec));
		cgip = cgiList[cid];
		cgip->handle = pHandle;
		cgip->stdIn = stdIn;
		cgip->stdOut = stdOut;
		cgip->cgiPath = cgiPath;
		cgip->argp = argp;
		cgip->envp = envp;
		cgip->wp = wp;
		cgip->fplacemark = 0;
		websTimeoutCancel(wp);
	}
/*
 *	Restore the current working directory after spawning child CGI
 */
 	gchdir(cwd);
	return 1;
}

/******************************************************************************/
/*
 *	Any entry in the cgiList need to be checked to see if it has
 */
void websCgiGatherOutput (cgiRec *cgip)
{
	gstat_t	sbuf;
	char_t	cgiBuf[FNAMESIZE];
#if defined(WIN32)
	errno_t	error;
#endif

	if ((gstat(cgip->stdOut, &sbuf) == 0) && 
		(sbuf.st_size > cgip->fplacemark)) {
		int fdout;
#if !defined(WIN32)
		fdout = gopen(cgip->stdOut, O_RDONLY | O_BINARY, 0444 );
#else
		error = _sopen_s(&fdout, cgip->stdOut, O_RDONLY | O_BINARY, _SH_DENYNO, 0444);
#endif

		/*
 *		Check to see if any data is available in the
 *		output file and send its contents to the socket.
 */
		if (fdout >= 0) {
			webs_t	wp = cgip->wp;
			int		nRead;
/*
 *			Write the HTTP header on our first pass
 */
			if (cgip->fplacemark == 0) {
				websWrite(wp, T("HTTP/1.0 200 OK\r\n"));
			}
			glseek(fdout, cgip->fplacemark, SEEK_SET);
			while ((nRead = gread(fdout, cgiBuf, FNAMESIZE)) > 0) {
				websWriteBlock(wp, cgiBuf, nRead);
				cgip->fplacemark += nRead;
			}
			gclose(fdout);
		}
	}
}



/******************************************************************************/
/*
 *	Any entry in the cgiList need to be checked to see if it has
 *	completed, and if so, process its output and clean up.
 */
void websCgiCleanup()
{
	cgiRec	*cgip;
	webs_t	wp;
	char_t	**ep;
	int		cid, nTries;
	for (cid = 0; cid < cgiMax; cid++) {
		if ((cgip = cgiList[cid]) != NULL) {
			wp = cgip->wp;
			websCgiGatherOutput (cgip);
			if (websCheckCgiProc(cgip->handle) == 0) {
/*
 *				We get here if the CGI process has terminated.  Clean up.
 */
				nTries = 0;
/*				
 *				Make sure we didn't miss something during a task switch.
 *				Maximum wait is 100 times 10 msecs (1 second).
 */
				while ((cgip->fplacemark == 0) && (nTries < 100)) {
					websCgiGatherOutput(cgip);
/*					
 *					There are some cases when we detect app exit 
 *					before the file is ready. 
 */
					if (cgip->fplacemark == 0) {
#ifdef WIN
						Sleep(10);
#endif /* WIN*/
					}
					nTries++;
				}
				if (cgip->fplacemark == 0) {
					websError(wp, 200, T("CGI generated no output"));
				} else {
					websDone(wp, 200);
				}
/*
 *				Remove the temporary re-direction files
 */
				gunlink(cgip->stdIn);
				gunlink(cgip->stdOut);
/*
 *				Free all the memory buffers pointed to by cgip.
 *				The stdin file name (wp->cgiStdin) gets freed as
 *				part of websFree().
 */
				cgiMax = hFree((void***) &cgiList, cid);
				for (ep = cgip->envp; ep != NULL && *ep != NULL; ep++) {
					bfreeSafe(B_L, *ep);
				}
				bfreeSafe(B_L, cgip->cgiPath);
				bfreeSafe(B_L, cgip->argp);
				bfreeSafe(B_L, cgip->envp);
				bfreeSafe(B_L, cgip->stdOut);
				bfreeSafe(B_L, cgip);
			}
		}
	}
}
/******************************************************************************/

/******************************************************************************/
/*	
	PLATFORM IMPLEMENTATIONS FOR CGI HELPERS
		websGetCgiCommName
		websLaunchCgiProc
		websCheckCgiProc
*/
/******************************************************************************/
#ifdef CE
/******************************************************************************/
/*
 *  Returns a pointer to an allocated qualified unique temporary file name.
 *  This filename must eventually be deleted with bfree().
 */

char_t *websGetCgiCommName()
{
/*
 *  tmpnam, tempnam, tmpfile not supported for CE 2.12 or lower.  The Win32 API
 *  GetTempFileName is scheduled to be part of CE 3.0.
 */
#if 0  
	char_t  *pname1, *pname2;

	pname1 = gtmpnam(NULL, T("cgi"));
	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
#endif
	return NULL;
}

/******************************************************************************/
/*
 *  Launch the CGI process and return a handle to it.
 *  CE note: This function is not complete.  The missing piece is the ability
 *  to redirect stdout.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
                      char_t *stdIn, char_t *stdOut)
{
	PROCESS_INFORMATION procinfo;       /*  Information about created proc   */
	DWORD               dwCreateFlags;
	char                *fulldir;
	BOOL                bReturn;
	int                 i, nLen;

/*
 *  Replace directory delimiters with Windows-friendly delimiters
 */
	nLen = gstrlen(cgiPath);
	for (i = 0; i < nLen; i++) {
		if (cgiPath[i] == '/') {
			cgiPath[i] = '\\';
		}
	}

	fulldir = NULL;
	dwCreateFlags = CREATE_NEW_CONSOLE;

/*
 *  CreateProcess returns errors sometimes, even when the process was
 *  started correctly.  The cause is not evident.  For now: we detect
 *  an error by checking the value of procinfo.hProcess after the call.
 */
	procinfo.hThread = NULL;
	bReturn = CreateProcess(
		cgiPath,            /*  Name of executable module        */
		NULL,               /*  Command line string              */
		NULL,               /*  Process security attributes      */
		NULL,               /*  Thread security attributes       */
		0,                  /*  Handle inheritance flag          */
		dwCreateFlags,      /*  Creation flags                   */
		NULL,               /*  New environment block            */
		NULL,               /*  Current directory name           */
		NULL,               /*  STARTUPINFO                      */
		&procinfo);         /*  PROCESS_INFORMATION              */

	if (bReturn == 0) {
		DWORD dw;
		dw = GetLastError();
		return -1;
	} else {
		CloseHandle(procinfo.hThread);
	}
	return (int) procinfo.dwProcessId;
}

/******************************************************************************/
/*
 *  Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */
int websCheckCgiProc(int handle)
{
	int     nReturn;
	DWORD   exitCode;

	nReturn = GetExitCodeProcess((HANDLE)handle, &exitCode);
/*
 *  We must close process handle to free up the window resource, but only
 *  when we're done with it.
 */
	if ((nReturn == 0) || (exitCode != STILL_ACTIVE)) {
		CloseHandle((HANDLE)handle);
		return 0;
	}

	return 1;
}
/******************************************************************************/
#endif /* CE */
/******************************************************************************/

/******************************************************************************/
#if defined(LINUX) || defined(LYNX) || defined(MACOSX) || defined(QNX4) 
/******************************************************************************/
#include <sys/wait.h>
/******************************************************************************/
/*
 *  Returns a pointer to an allocated qualified unique temporary file name.
 *  This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
	char_t  *pname1, *pname2;

	pname1 = tempnam(NULL, T("cgi"));
	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
}

/******************************************************************************/
/*
 *  Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
                      char_t *stdIn, char_t *stdOut)
{
	int pid, fdin, fdout, hstdin, hstdout, rc;

	fdin = fdout = hstdin = hstdout = rc = -1;
	if ((fdin = open(stdIn, O_RDWR | O_CREAT, 0666)) < 0 ||
			(fdout = open(stdOut, O_RDWR | O_CREAT, 0666)) < 0 ||
			(hstdin = dup(0)) == -1 ||
			(hstdout = dup(1)) == -1 ||
			dup2(fdin, 0) == -1 ||
			dup2(fdout, 1) == -1) {
		goto DONE;
	}

	rc = pid = fork();
	if (pid == 0) {
/*
 *		if pid == 0, then we are in the child process
 */
		if (execve(cgiPath, argp, envp) == -1) {
			printf("content-type: text/html\n\n"
				"Execution of cgi process failed\n");
		}
		exit (0);
	}

DONE:
	if (hstdout >= 0) {
		dup2(hstdout, 1);
		close(hstdout);
	}
	if (hstdin >= 0) {
		dup2(hstdin, 0);
		close(hstdin);
	}
	if (fdout >= 0) {
		close(fdout);
	}
	if (fdin >= 0) {
		close(fdin);
	}
	return rc;
}

/******************************************************************************/
/*
 *  Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */
int websCheckCgiProc(int handle)
{
/*
 *  Check to see if the CGI child process has terminated or not yet.
 */
	if (waitpid(handle, NULL, WNOHANG) == handle) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************/
#endif /* LINUX || LYNX || MACOSX || QNX4 */
/******************************************************************************/

/******************************************************************************/
#ifdef NW
/******************************************************************************/
/******************************************************************************/
/*
 * Returns a pointer to an allocated qualified unique temporary file name.
 * This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
	char_t   *pname;

	pname = bstrdup(B_L, tmpnam( NULL ) );
	return pname;
}
/******************************************************************************/
/*
 * Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
                 char_t *stdIn, char_t *stdOut)
{
	int   pid, fdin, fdout, hstdin, hstdout, rc;

	fdin = fdout = -1;
	if ((fdin = open(stdIn, O_RDWR | O_CREAT, 0666)) < 0 ||
			(fdout = open(stdOut, O_RDWR | O_CREAT, 0666)) < 0 )
		goto DONE;

/*  COMMENTED BLOCK ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù
 *     printf( "[%s]\n[", cgiPath ) ;
 *  {
 *     char \**x = argp;
 *     while( *argp ) printf( "%s-", *argp ++ ) ;
 * 
 *  }
 *     printf( "]\n" ) ;
 *  END COMMENTS ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù
 */

	rc = spawnvp( P_NOWAIT, cgiPath, argp ) ;

	if( rc != 0) {
		exit (0);
	}

DONE:
	if (fdout >= 0) {
		close(fdout);
	}
	if (fdin >= 0) {
		close(fdin);
	}
	return rc;
}

/******************************************************************************/
/*
 * Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */
int websCheckCgiProc(int handle)
{
/*
 * Check to see if the CGI child process has terminated or not yet.
 */
	if( handle ) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************/
#endif /* NW */
/******************************************************************************/

/******************************************************************************/
#ifdef VXWORKS
/******************************************************************************/
static void vxWebsCgiEntry(void *entryAddr(int argc, char_t **argv),
				char_t **argv, char_t **envp, char_t *stdIn, char_t *stdOut);
/******************************************************************************/
/*
 *  Returns a pointer to an allocated qualified unique temporary file name.
 *  This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
    char_t  *tname, buf[FNAMESIZE];

	fmtAlloc(&tname,FNAMESIZE, T("%s/%s"), ggetcwd(buf, FNAMESIZE),
		tmpnam(NULL));
	return tname;
}

/******************************************************************************/
/*
 *  Launch the CGI process and return a handle to it. Process spawning
 *  is not supported in VxWorks.  Instead, we spawn a "task".  A major
 *  difference is that we have to know the entry point for the taskSpawn
 *  API.  Also the module may have to be loaded before being executed;
 *  it may also be part of the OS image, in which case it cannot be
 *  loaded or unloaded.  The following sequence is used:
 *  1. If the module is already loaded, unload it from memory.
 *  2. Search for a query string keyword=value pair in the environment
 *      variables where the keyword is cgientry.  If found use its value
 *      as the the entry point name.  If there is no such pair set
 *      the entry point name to the default: basename_cgientry, where
 *      basename is the name of the cgi file without the extension.  Use
 *      the entry point name in a symbol table search for that name to
 *      use as the entry point address.  If successful go to step 5.
 *  3. Try to load the module into memory.  If not successful error out.
 *  4. If step 3 is successful repeat the entry point search from step
 *      2.  If the entry point exists, go to step 5.  If it does not,
 *      error out.
 *  5. Use taskSpawn to start a new task which uses vxWebsCgiEntry
 *      as its starting point.  The five arguments to vxWebsCgiEntry
 *      will be the user entry point address, argp, envp, stdIn
 *      and stdOut.  vxWebsCgiEntry will convert argp to an argc
 *      argv pair to pass to the user entry, it will initialize the
 *      task environment with envp, it will open and redirect stdin
 *      and stdout to stdIn and stdOut, and then it will call the
 *      user entry.
 *  6.  Return the taskSpawn return value.
 */
int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
						char_t *stdIn, char_t *stdOut)
{
	SYM_TYPE    ptype;
	char_t      *p, *basename, *pEntry, *pname, *entryAddr, **pp;
	int         priority, rc, fd;

/*
 *  Determine the basename, which is without path or the extension.
 */
	if ((int)(p = gstrrchr(cgiPath, '/') + 1) == 1) {
		p = cgiPath;
	}
	basename = bstrdup(B_L, p);
	if ((p = gstrrchr(basename, '.')) != NULL) {
		*p = '\0';
	}

/*
 *  Unload the module, if it is already loaded.  Get the current task
 *  priority.
 */
	unld(cgiPath, 0);
	taskPriorityGet(taskIdSelf(), &priority);
	rc = fd = -1;

/*
 *  Set the entry point symbol name as described above.  Look for an already
 *  loaded entry point; if it exists, spawn the task accordingly.
 */
	for (pp = envp, pEntry = NULL; pp != NULL && *pp != NULL; pp++) {
		if (gstrncmp(*pp, T("cgientry="), 9) == 0) {
			pEntry = bstrdup(B_L, *pp + 9);
			break;
		}
	}
	if (pEntry == NULL) {
		fmtAlloc(&pEntry, LF_PATHSIZE, T("%s_%s"), basename, T("cgientry"));
	}
	entryAddr = 0;
	if (symFindByName(sysSymTbl, pEntry, &entryAddr, &ptype) == -1) {
		fmtAlloc(&pname, VALUE_MAX_STRING, T("_%s"), pEntry);
		symFindByName(sysSymTbl, pname, &entryAddr, &ptype);
		bfreeSafe(B_L, pname);
	}
	if (entryAddr != 0) {
		rc = taskSpawn(pEntry, priority, 0, 20000, (void *)vxWebsCgiEntry,
			(int)entryAddr, (int)argp, (int)envp, (int)stdIn, (int)stdOut,
			0, 0, 0, 0, 0);
		goto DONE;
	}

/*
 *  Try to load the module.
 */
	if ((fd = gopen(cgiPath, O_RDONLY | O_BINARY, 0666)) < 0 ||
		loadModule(fd, LOAD_GLOBAL_SYMBOLS) == NULL) {
		goto DONE;
	}
	if ((symFindByName(sysSymTbl, pEntry, &entryAddr, &ptype)) == -1) {
		fmtAlloc(&pname, VALUE_MAX_STRING, T("_%s"), pEntry);
		symFindByName(sysSymTbl, pname, &entryAddr, &ptype);
		bfreeSafe(B_L, pname);
	}
	if (entryAddr != 0) {
		rc = taskSpawn(pEntry, priority, 0, 20000, (void *)vxWebsCgiEntry,
			(int)entryAddr, (int)argp, (int)envp, (int)stdIn, (int)stdOut,
			0, 0, 0, 0, 0);
	}

DONE:
	if (fd != -1) {
		gclose(fd);
	}
	bfree(B_L, basename);
	bfree(B_L, pEntry);
	return rc;
}

/******************************************************************************/
/*
 *  This is the CGI process wrapper.  It will open and redirect stdin
 *  and stdout to stdIn and stdOut.  It converts argv to an argc, argv
 *  pair to pass to the user entry. It initializes the task environment
 *  with envp strings.  Then it will call the user entry.
 */
static void vxWebsCgiEntry(void *entryAddr(int argc, char_t **argv),
				char_t **argp, char_t **envp, char_t *stdIn, char_t *stdOut)
{
	char_t  **p;
	int     argc, taskId, fdin, fdout;

/*
 *  Open the stdIn and stdOut files and redirect stdin and stdout
 *  to them.
 */
	taskId = taskIdSelf();
	if ((fdout = gopen(stdOut, O_RDWR | O_CREAT, 0666)) < 0 &&
			(fdout = creat(stdOut, O_RDWR)) < 0) {
		exit(0);
	}
	ioTaskStdSet(taskId, 1, fdout);

	if ((fdin = gopen(stdIn, O_RDONLY | O_CREAT, 0666)) < 0 &&
			(fdin = creat(stdIn, O_RDWR)) < 0) {
		printf("content-type: text/html\n\n"
				"Can not create CGI stdin to %s\n", stdIn);
		gclose(fdout);
		exit (0);
	}
	ioTaskStdSet(taskId, 0, fdin);

/*
 *  Count the number of entries in argv
 */
	for (argc = 0, p = argp; p != NULL && *p != NULL; p++, argc++) {
	}

/*
 *  Create a private envirnonment and copy the envp strings to it.
 */
	if (envPrivateCreate(taskId, -1) != OK) {
		printf("content-type: text/html\n\n"
			"Can not create CGI environment space\n");
		gclose(fdin);
		gclose(fdout);
		exit (0);
	}
	for (p = envp; p != NULL && *p != NULL; p++) {
		putenv(*p);
	}

/*
 *  Call the user entry.
 */
	(*entryAddr)(argc, argp);

/*
 *  The user code should return here for cleanup.
 */

	envPrivateDestroy(taskId);
	gclose(fdin);
	gclose(fdout);
	exit(0);
}

/******************************************************************************/
/*
 *  Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc(int handle)
{
	STATUS stat;
/*
 *  Verify the existence of a VxWorks task
 */
	stat = taskIdVerify(handle);

	if (stat == OK) {
		return 1;
	} else {
		return 0;
	}
}
/******************************************************************************/
#endif /* VXWORKS */
/******************************************************************************/

/******************************************************************************/
#ifdef WIN 
/******************************************************************************/
/******************************************************************************/
/*
 *	Convert a table of strings into a single block of memory.
 *	The input table consists of an array of null-terminated strings,
 *	terminated in a null pointer.
 *	Returns the address of a block of memory allocated using the balloc() 
 *	function.  The returned pointer must be deleted using bfree().
 *	Returns NULL on error.
 */

static unsigned char *tableToBlock(char **table)
{
    unsigned char	*pBlock;		/*  Allocated block */
    char			*pEntry;		/*  Pointer into block      */
    size_t			sizeBlock;		/*  Size of table           */
    int				index;			/*  Index into string table */

    a_assert(table);

/*  
 *	Calculate the size of the data block.  Allow for final null byte. 
 */
    sizeBlock = 1;                    
    for (index = 0; table[index]; index++) {
        sizeBlock += strlen(table[index]) + 1;
	}

/*
 *	Allocate the data block and fill it with the strings                   
 */
    pBlock = balloc(B_L, sizeBlock);

	if (pBlock != NULL) {
		pEntry = (char *) pBlock;

        for (index = 0; table[index]; index++) {
			strcpy(pEntry, table[index]);
			pEntry += strlen(pEntry) + 1;
		}

/*		
 *		Terminate the data block with an extra null string                
 */
		*pEntry = '\0';              
	}

	return pBlock;
}
/******************************************************************************/
/*
 *  Returns a pointer to an allocated qualified unique temporary file name.
 *  This filename must eventually be deleted with bfree().
 */

char_t *websGetCgiCommName()
{
	char_t  *pname1, *pname2;

	pname1 = tempnam(NULL, T("cgi"));
	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
}
/******************************************************************************/
/*
 *  Create a temporary stdout file and launch the CGI process.
 *  Returns a handle to the spawned CGI process.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
                      char_t *stdIn, char_t *stdOut)
{
	STARTUPINFO         newinfo;
	SECURITY_ATTRIBUTES security;
	PROCESS_INFORMATION procinfo;       /*  Information about created proc   */
	DWORD               dwCreateFlags;
	char_t              *cmdLine;
	char_t              **pArgs;
	BOOL                bReturn;
	int                 i, nLen;
	unsigned char       *pEnvData;

/*
 *  Replace directory delimiters with Windows-friendly delimiters
 */
	nLen = gstrlen(cgiPath);
	for (i = 0; i < nLen; i++) {
		if (cgiPath[i] == '/') {
			cgiPath[i] = '\\';
		}
	}
/*
 *  Calculate length of command line
 */
	nLen = 0;
	pArgs = argp;
	while (pArgs && *pArgs && **pArgs) {
		nLen += gstrlen(*pArgs) + 1;
		pArgs++;
	}
/*
 *  Construct command line
 */
	cmdLine = balloc(B_L, sizeof(char_t) * nLen);
	a_assert (cmdLine);
	gstrcpy(cmdLine, "");

	pArgs = argp;
	while (pArgs && *pArgs && **pArgs) {
		gstrcat(cmdLine, *pArgs);
		gstrcat(cmdLine, T(" "));
		pArgs++;
	}

/*
 *  Create the process start-up information
 */
	memset (&newinfo, 0, sizeof(newinfo));
	newinfo.cb          = sizeof(newinfo);
	newinfo.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	newinfo.wShowWindow = SW_HIDE;
	newinfo.lpTitle     = NULL;

/*
 *  Create file handles for the spawned processes stdin and stdout files
 */
	security.nLength = sizeof(SECURITY_ATTRIBUTES);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;

/*
 *  Stdin file should already exist.
 */
	newinfo.hStdInput = CreateFile(stdIn, GENERIC_READ,
		FILE_SHARE_READ, &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
/*
 *  Stdout file is created and file pointer is reset to start.
 */
	newinfo.hStdOutput = CreateFile(stdOut, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ + FILE_SHARE_WRITE, &security, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer (newinfo.hStdOutput, 0, NULL, FILE_END);

/*
 *  Stderr file is set to Stdout.
 */
	newinfo.hStdError = newinfo.hStdOutput;

	dwCreateFlags = CREATE_NEW_CONSOLE;
	pEnvData = tableToBlock(envp);

/*
 *  CreateProcess returns errors sometimes, even when the process was
 *  started correctly.  The cause is not evident.  For now: we detect
 *  an error by checking the value of procinfo.hProcess after the call.
 */
	procinfo.hProcess = NULL;
	bReturn = CreateProcess(
		NULL,               /*  Name of executable module        */
		cmdLine,            /*  Command line string              */
		NULL,               /*  Process security attributes      */
		NULL,               /*  Thread security attributes       */
		TRUE,               /*  Handle inheritance flag          */
		dwCreateFlags,      /*  Creation flags                   */
        pEnvData,           /*  New environment block            */
		NULL,               /*  Current directory name           */
		&newinfo,           /*  STARTUPINFO                      */
		&procinfo);         /*  PROCESS_INFORMATION              */

	if (procinfo.hThread != NULL)  {
		CloseHandle(procinfo.hThread);
	}

	if (newinfo.hStdInput) {
		CloseHandle(newinfo.hStdInput);
	}

	if (newinfo.hStdOutput) {
		CloseHandle(newinfo.hStdOutput);
	}

	bfree(B_L, pEnvData);
	bfree(B_L, cmdLine);

	if (bReturn == 0) {
		return -1;
	} else {
		return (int) procinfo.hProcess;
	}
}

/******************************************************************************/
/*
 *  Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc(int handle)
{
	int     nReturn;
	DWORD   exitCode;

	nReturn = GetExitCodeProcess((HANDLE)handle, &exitCode);
/*
 *  We must close process handle to free up the window resource, but only
 *  when we're done with it.
 */
	if ((nReturn == 0) || (exitCode != STILL_ACTIVE)) {
		CloseHandle((HANDLE)handle);
		return 0;
	}

	return 1;
}

/******************************************************************************/
#endif /* WIN */
/******************************************************************************/
