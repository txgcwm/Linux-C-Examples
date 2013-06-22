#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "desktop-entry-util.h"
#include "desktop-list.h"

typedef int Myfunc(const char *, const struct stat *);

struct list_head head;

LIST_HEAD(head);

static char *path_alloc(int *sizep)
{
#define SUSV3	200112L
#define	PATH_MAX_GUESS	1024
					
#ifdef	PATH_MAX
	static int pathmax = PATH_MAX;
#else
	static int pathmax = 0;
#endif
	static long posix_version = 0;
	char *ptr = NULL;
	int size = 0;

	if (posix_version == 0)
		posix_version = sysconf(_SC_VERSION);

	if (pathmax == 0) {			/* first time through */
		errno = 0;
		if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
			if (errno == 0)
				pathmax = PATH_MAX_GUESS;	/* it's indeterminate */
			else {
				printf("pathconf error for _PC_PATH_MAX");
				return NULL;
			}
		} else {
			pathmax++;			/* add one since it's relative to root */
		}
	}
	if (posix_version < SUSV3)
		size = pathmax + 1;
	else
		size = pathmax;

	if ((ptr = malloc(size)) == NULL) {
		printf("malloc error for pathname\n");
		return NULL;
	}

	if (sizep != NULL)
		*sizep = size;

	return ptr;
}

static int myfunc(const char *pathname, const struct stat *statptr)
{
	struct DesktopEntry *entry = NULL;

	switch (statptr->st_mode & S_IFMT) {
	case S_IFREG:
	case S_IFLNK:
		{
			printf("%s\n", pathname);
			entry = (struct DesktopEntry *)malloc(sizeof(struct DesktopEntry));
			if(entry == NULL)
				return -1;
			memset(entry, 0, sizeof(struct DesktopEntry));
			desktop_read_file(entry, pathname);
			desktop_list_add_tail(entry, &head);
		}
		break;
	case S_IFDIR:
	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
	case S_IFSOCK:
		break;
	default:
		printf("unknown pathname %s\n", pathname);
	}

	return 0;
}

static int dopath(char *fullpath, Myfunc * func)
{
	struct stat statbuf;
	struct dirent *dirp = NULL;
	DIR *dp = NULL;
	int ret = -1;
	char *ptr = NULL;

	if (lstat(fullpath, &statbuf) < 0)
		return ret;
	if (S_ISDIR(statbuf.st_mode) == 0)
		return (func(fullpath, &statbuf));

	if ((ret = func(fullpath, &statbuf)) != 0)
		return ret;

	ptr = fullpath + strlen(fullpath);
	*ptr++ = '/';
	*ptr = 0;

	if ((dp = opendir(fullpath)) == NULL)
		return ret;

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;			

		strcpy(ptr, dirp->d_name);

		if ((ret = dopath(fullpath, func)) != 0)
			break;				
	}
	ptr[-1] = 0;				

	if (closedir(dp) < 0) {
		printf("can't close directory %s", fullpath);
	}

	return ret;
}

static int myftw(char *pathname, Myfunc * func)
{
	static char *fullpath = NULL;
	int len = 0;

	fullpath = path_alloc(&len);	
	strncpy(fullpath, pathname, len);
	fullpath[len - 1] = 0;

	return (dopath(fullpath, func));
}

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 2) {
		printf("usage:  ftw  <starting-pathname>");
		return 0;
	}

	ret = myftw(argv[1], myfunc);
	desktop_list_items(&head);

	exit(ret);
}
