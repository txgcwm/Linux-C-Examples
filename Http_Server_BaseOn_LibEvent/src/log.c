/******************************************************************
 *
 *
 *
 *
 *
 *
 ******************************************************************/

#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

/*Max length per message*/
#define MAXLENGTH 	4096

typedef struct {
	int fd;
	unsigned long flags;
	const char *domain;
} Logger;

static Logger logger = { -1, 0, "" };

static char *currentTimeString(void)
{
	time_t rawtime;
	struct tm *timeinfo;
	char *buffer = NULL;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	buffer = (char *)malloc(sizeof(char) * 81);
	strftime(buffer, 80, "%c", timeinfo);

	return buffer;
}

bool Log_Init(const char *domain, int logflags)
{
	int openFlags = O_CREAT | O_WRONLY | O_APPEND;;
	mode_t filePerms = S_IRUSR | S_IWUSR;

	if (domain != NULL)
		logger.domain = domain;

	logger.flags = logflags;

	if (logger.domain && (strlen(logger.domain) > 0)
		&& ((logger.flags & LOGFLAG_FILE) == LOGFLAG_FILE)) {

		logger.fd = open(logger.domain, openFlags, filePerms);
		if (logger.fd < 0) {
			printf("Could not open file for logging.");
		}
	}

	return true;
}

bool Log_Close()
{

	if (logger.fd > 0 && close(logger.fd)) {
		printf("Could not close log file.");
		return false;
	}
	logger.fd = -1;

	return true;
}

void Log_Message(const char *info, const char *format, ...)
{
	va_list args;
	char timeformat[MAXLENGTH];
	char buffer[MAXLENGTH];
	char *timeString = NULL;
	size_t bufferSize = 0;

	va_start(args, format);

	timeString = currentTimeString();
	snprintf(timeformat, MAXLENGTH, "[%s] %s: %s", timeString, info, format);
	bufferSize = vsnprintf(buffer, MAXLENGTH, timeformat, args);

	if ((logger.flags & LOGFLAG_STDOUT) == LOGFLAG_STDOUT) {
		printf("%s", buffer);
		fflush(stdout);
	} else {
		/*Write the logs to file */
		if ((logger.fd > 0) && (logger.flags & LOGFLAG_FILE) == LOGFLAG_FILE) {
			if (write(logger.fd, buffer, bufferSize) < 0) {

			}
		} else {
			printf("%s", buffer);
			fflush(stdout);
		}
	}

	free(timeString);
	va_end(args);

	return;
}
