/******************************************************************
 *
 *
 *
 *
 * author: txgcwm
 *
 ******************************************************************/

#ifndef _LOG_H_
#define _LOG_H_

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>

#define LOG_DEBUG_STRING "Debug"
#define LOG_TRACE_STRING "Trace"
#define LOG_ERROR_STRING "Error"
#define LOG_INFO_STRING  "Info"
#define LOG_WARN_STRING  "Warn"

typedef enum {
	LOGFLAG_FILE = 1 << 0,		/* write log to a file with the name specified by domain */
	LOGFLAG_STDOUT = 1 << 1,	/* write log to stdout */
} LOGFLAGS;

bool Log_Init(const char *domain, int logflags);
bool Log_Close(void);

#define LOG_ERRORS(format, args...)		Log_Message(LOG_ERROR_STRING, format, ##args)
#define LOG_WARNS(format, args...) 		Log_Message(LOG_WARN_STRING, format, ##args)
#define LOG_INFOS(format, args...) 		Log_Message(LOG_INFO_STRING, format, ##args)
#define LOG_DEBUGS(format, args...)		Log_Message(LOG_DEBUG_STRING, format, ##args)
#define LOG_TRACES(format, args...)		Log_Message(LOG_TRACE_STRING, format, ##args)

#endif // LOG_H
