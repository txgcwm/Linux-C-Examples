/*
 * webcomp -- Compile web pages into C source
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * Release $Name: WEBSERVER-2-5 $
 */

/******************************** Description *********************************/

/*
 *	Usage: webcomp prefix filelist >webrom.c
 *
 *	filelist is a file containing the pathnames of all web pages
 *	prefix is a path prefix to remove from all the web page pathnames
 *	webrom.c is the resulting C source file to compile and link.
 */

/********************************* Includes ***********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct stat stat_t;

#ifndef O_BINARY
#define O_BINARY        0
#endif /* O_BINARY */
#ifndef FNAMESIZE
#define FNAMESIZE           254         /* Max length of file names */
#endif /* FNAMESIZE */

/**************************** Forward Declarations ****************************/

static int 	compile(char *fileList, char *prefix);
static void usage();

/*********************************** Code *************************************/
/*
 *	Main program
 */

int main(int argc, char* argv[])
{
	char		*fileList, *prefix;

	fileList = NULL;

	if (argc != 3) {
		usage();
	}

	prefix = argv[1];
	fileList = argv[2];

	if (compile(fileList, prefix) < 0) {
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Output usage message
 */

static void usage()
{
	fprintf(stderr, "usage: webcomp prefix filelist >output.c\n\
    filelist is a file containing the pathnames of all web pages\n\
    prefix is a path prefix to remove from all the web page pathnames\n\
    output.c is the resulting C source file to compile and link.\n");

	exit(2);
}

/******************************************************************************/
/*
 *	Compile the web pages
 */

static int compile(char *fileList, char *prefix)
{
	stat_t			sbuf;
	FILE			*lp;
	time_t			now;
	char			file[FNAMESIZE];
	char			*cp, *sl;
	char			buf[512];
	unsigned char	*p;
	int				j, i, len, fd, nFile;

/*
 *	Open list of files
 */
	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}

	time(&now);
	fprintf(stdout, "/*\n * webrom.c -- Compiled Web Pages\n *\n");
	fprintf(stdout, " * Compiled by GoAhead WebCompile: %s */\n\n", 
		ctime(&now));
	fprintf(stdout, "#include \"wsIntrn.h\"\n\n");
	fprintf(stdout, "#ifndef WEBS_PAGE_ROM\n");
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");
	fprintf(stdout, "\t{ 0, 0, 0 }\n};\n");
	fprintf(stdout, "#else\n");

/*
 *	Open each input file and compile each web page
 */
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = (unsigned char *)strchr(file, '\n')) || 
				(p = (unsigned char *)strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		} 
		if ((fd = open(file, O_RDONLY | O_BINARY)) < 0) {
			fprintf(stderr, "Can't open file %s\n", file);
			return -1;
		}
		fprintf(stdout, "/* %s */\n", file);
		fprintf(stdout, "static unsigned char p%d[] = {\n", nFile);

		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			p = (unsigned char*)buf;
			for (i = 0; i < len; ) {
				fprintf(stdout, "\t");
				for (j = 0; p<(unsigned char*)(&buf[len]) && j < 16; j++, p++) {
					fprintf(stdout, "%3d,", *p);
				}
				i += j;
				fprintf(stdout, "\n");
			}
		}
		fprintf(stdout, "\t0 };\n\n");

		close(fd);
		nFile++;
	}
	fclose(lp);

/*
 *	Now output the page index
 */
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");

	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = (unsigned char *)strchr(file, '\n')) || 
				(p = (unsigned char *)strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
/*
 *		Remove the prefix and add a leading "/" when we print the path
 */
		if (strncmp(file, prefix, strlen(prefix)) == 0) {
			cp = &file[strlen(prefix)];
		} else {
			cp = file;
		}
		while((sl = strchr(file, '\\')) != NULL) {
			*sl = '/';
		}
		if (*cp == '/') {
			cp++;
		}

		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			fprintf(stdout, "\t{ T(\"/%s\"), 0, 0 },\n", cp);
			continue;
		}
		fprintf(stdout, "\t{ T(\"/%s\"), p%d, %d },\n", cp, nFile, 
			sbuf.st_size);
		nFile++;
	}
	fclose(lp); 
	
	fprintf(stdout, "\t{ 0, 0, 0 }\n");
	fprintf(stdout, "};\n");
	fprintf(stdout, "#endif /* WEBS_PAGE_ROM */\n");

	fclose(lp);
	fflush(stdout);
	return 0;
}

/******************************************************************************/

