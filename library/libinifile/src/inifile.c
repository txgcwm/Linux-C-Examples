/* libinifile - library for parsing ini-style configuration files.
 * Copyright (C) 2008  Anders Lövgren
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ---
 * Send questions to: Anders Lövgren <lespaul@algonet.se>
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_STDAFX_H
# include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "inifile.h"
#include "common.h"

/*
 * Initilize the parser. Returns 0 if successful and -1 on
 * failure. Use inifile_get_error() to get last error.
 */
int inifile_init(struct inifile *inf, const char *conf)
{
	memset(inf, 0, sizeof(struct inifile));
	
	inf->file = conf;	

	inf->fs = fopen(inf->file, "r");
	if(!inf->fs) {
		inifile_set_error(inf, 0, 0, "failed open %s", inf->file);
		return -1;
	}
	
	inf->entry = malloc(sizeof(struct inient));
	if(!inf->entry) {
		inifile_set_error(inf, 0, 0, "failed alloc memory");
		return -1;
	}
	inf->entry->sect = NULL;
	inf->entry->key = NULL;
	inf->entry->val = NULL;

	inf->str = NULL;
	inf->size = 0;
	inf->len = 0;

	inf->options = INIFILE_DEFAULT_OPTIONS;
	
	inf->comment = malloc(strlen(INIFILE_DEFIDS_COMMENT) + 1);
	if(!inf->comment) {
		inifile_set_error(inf, 0, 0, "failed alloc memory");
		return -1;
	}
	strcpy(inf->comment, INIFILE_DEFIDS_COMMENT);
	
	inf->assign = malloc(strlen(INIFILE_DEFIDS_ASSIGN) + 1);
	if(!inf->assign) {
		inifile_set_error(inf, 0, 0, "failed alloc memory");
		return -1;
	}
	strcpy(inf->assign, INIFILE_DEFIDS_ASSIGN);
	
	return 0;
}

/*
 * Return next parsed ini-file entry or NULL if last call 
 * failed (including end of file). Use inifile_get_error() 
 * to check the error condition.
 */
const struct inient * inifile_parse(struct inifile *inf)
{	
	return (struct inient *)parser_get_next(inf);
}

/*
 * Set parser option.
 */
int inifile_set_option(struct inifile *inf, int option, const void *value)
{
	if(option == INIFILE_CHECK_SYNTAX  ||
	   option == INIFILE_ALLOW_QUOTE   ||
	   option == INIFILE_ASSIGN_INSIDE ||
	   option == INIFILE_ALLOW_MULTILINE ||
	   option == INIFILE_COMPACT_MLINE) {
		const int *optval = (const int *)value;
		if(*optval) {
			inf->options |= option;
		} else {
			inf->options &= ~option;
		}		
	} else if(option == INIFILE_CHARS_COMMENT) {
		const char *optval = (const char *)value;
		inf->comment = realloc(inf->comment, strlen(optval) + 1);
		if(!inf->comment) {
			inifile_set_error(inf, 0, 0, "failed alloc memory");
			return -1;
		}
		strcpy(inf->comment, optval);
	} else if(option == INIFILE_CHARS_ASSIGN) {
		const char *optval = (const char *)value;
		inf->assign = realloc(inf->assign, strlen(optval) + 1);
		if(!inf->assign) {
			inifile_set_error(inf, 0, 0, "failed alloc memory");
			return -1;
		}
		strcpy(inf->assign, optval);
	} else {
		inifile_set_error(inf, 0, 0, "unknown option %d", option);
		return -1;
	}
	
	return 0;
}
	
/*
 * Get parser option.
 */
int inifile_get_option(struct inifile *inf, int option, void *value)
{	
	if(option == INIFILE_CHECK_SYNTAX    ||
	   option == INIFILE_ALLOW_QUOTE     ||
	   option == INIFILE_ASSIGN_INSIDE   ||
	   option == INIFILE_ALLOW_MULTILINE ||
	   option == INIFILE_COMPACT_MLINE) {
		int *optval = (int *)value;
		*optval = (inf->options & option) ? 1 : 0;
	} else if(option == INIFILE_CHARS_COMMENT) {
		char **optval = (char **)value;
		*optval = inf->comment;
	} else if(option == INIFILE_CHARS_ASSIGN) {
		char **optval = (char **)value;
		*optval = inf->assign;
	} else {
		inifile_set_error(inf, 0, 0, "unknown option %d", option);
		return -1;
	}
	
	return 0;
}

/*
 * Release memory allocated by the parser.
 */
void inifile_free(struct inifile *inf)
{
	if(inf->fs) {
		fclose(inf->fs);
		inf->fs = NULL;
	}
	if(inf->entry) {
		if(inf->entry->sect) {
			free(inf->entry->sect);
			inf->entry->sect = NULL;
		}
		if(inf->entry->key) {
			free(inf->entry->key);
			inf->entry->key = NULL;
		}
		if(inf->entry->val) {
			free(inf->entry->val);
			inf->entry->val = NULL;
		}
		free(inf->entry);
		inf->entry = NULL;
	}
	if(inf->error) {
		free(inf->error);
		inf->error = NULL;
	}
	if(inf->str) {
		free(inf->str);
		inf->str = NULL;
		inf->len = 0;
		inf->size = 0;
	}	
	if(inf->comment) {
		free(inf->comment);
		inf->comment = NULL;
	}
	if(inf->assign) {
		free(inf->assign);
		inf->assign = NULL;
	}
}

/*
 * Get last error or NULL if no error occured.
 */
const struct inierr * inifile_get_error(struct inifile *inf)
{
	return inf->error;
}

/*
 * Write error message.
 */
void inifile_set_error(struct inifile *inf, 
		       unsigned int line, 
		       unsigned int pos,
		       const char *fmt, ...)
{
	int bytes;
	int size = 100;
	va_list ap;
	char *buff;
	
	if(!inf->error) {
		inf->error = malloc(sizeof(struct inierr));
		if(!inf->error) {
			return;
		}
		inf->error->msg = malloc(size);
		if(!inf->error->msg) {
			free(inf->error);
			inf->error = NULL;
			return;
		}
	}
	
	inf->error->line = line;
	inf->error->pos  = pos;
		
	while(1) {
		/* 
		 * Try to print in the allocated space. 
		 */		
		va_start(ap, fmt);
		bytes = vsnprintf(inf->error->msg, size, fmt, ap);
		va_end(ap);
		
		/* 
		 * If that worked then we are done.
		 */
		if(bytes > -1 && bytes < size) {
			break;
		}
		
		/* 
		 * Realloc memory and try again.
		 */
		if(bytes > -1) {              /* glibc 2.1 */
			size = bytes + 1;     /* precisely what is needed */
		} else {                      /* glibc 2.0 */
			size *= 2;            /* twice the old size */
		}
		if((buff = realloc(inf->error->msg, size)) == NULL) {
			free(inf->error->msg);
			inf->error->msg = NULL;
			break;
		} else {
			inf->error->msg = buff;
		}
	}	
}

/*
 * Clear error object.
 */
void inifile_clear_error(struct inifile *inf)
{
	if(inf->error) {
		if(inf->error->msg) {
			free(inf->error->msg);
		}
		free(inf->error);
		inf->error = NULL;
	}
}

/*
 * Remove leading and trailing whitespace characters from
 * string pointed to by str.
 */
char * inifile_trim_str(char *str)
{
	size_t start, end, length, i;
	
	if(!str || str[0] == '\0') {
		return str;
	}
	start = 0;
	end = strlen(str);
	
	while(isspace(str[end - 1])) {
		--end;
	}
	while(isspace(str[start])) {
		++start;
	}
	if(start > end) {
		str[0] = '\0';
		return str;
	}
	length = end - start;

	/* fprintf(stderr, "start=%d, end=%d, length=%d, str='%s'\n",  */
	/* 	start, end, length, str); */
	
	/*
	 * Note: we can't use memmove because it corrupts the pointer.
	 */
	
	for(i = 0; i < length; ++i) {
		str[i] = str[i + start];
	}
	for(i = end - start; i <= end; ++i) {
		str[i] = '\0';
	}

	return str;
}
