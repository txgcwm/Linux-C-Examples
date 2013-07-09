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

#ifndef __LIBINIFILE_INIFILE_H__
#define __LIBINIFILE_INIFILE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif 
	
/*
 * An ini-file entry.
 */
struct inient
{
	unsigned int line;      /* line number. */
	char *sect;		/* section name. */
	char *key;		/* entry keyword. */
	char *val;		/* entry value. */
};

/*
 * Parse error or libc error.
 */
struct inierr
{
	unsigned int line;
	unsigned int pos;
	char *msg;
};

/*
 * Used by parser.
 */
struct inifile 
{
	FILE *fs;               /* opened ini-file */
	const char *file;       /* ini-file path */
	struct inient *entry;   /* current entry */
	struct inierr *error;   /* last error */
	char *str;		/* parse buffer */
	size_t size;            /* parse buffer size */
	ssize_t len;            /* buffer string length */
	int options;            /* parse options */
	char *comment;          /* comment chars */
	char *assign;           /* assign chars */
};

/*
 * Options for inifile_xxx_option().
 */
#define INIFILE_CHECK_SYNTAX      1    /* syntax check */
#define INIFILE_ALLOW_QUOTE       2    /* allow quoted strings */
#define INIFILE_ASSIGN_INSIDE     4    /* allow s1=s2 inside values */
#define INIFILE_ALLOW_MULTILINE   8    /* allow multiline ('\') */
#define INIFILE_COMPACT_MLINE    16    /* eat whitespace in multiline */
	
#define INIFILE_CHARS_COMMENT   256    /* get/set comment chars */
#define INIFILE_CHARS_ASSIGN    512    /* get/set assignment chars */
	
#define INIFILE_DEFAULT_OPTIONS (INIFILE_CHECK_SYNTAX | INIFILE_ALLOW_QUOTE | INIFILE_COMPACT_MLINE)

/*
 * Default comment chars and assignment chars.
 */
#define INIFILE_DEFIDS_COMMENT "#" /* default comment chars */
#define INIFILE_DEFIDS_ASSIGN  "=" /* default assignment chars */

/*
 * Initilize the parser. Returns 0 if successful and -1 on
 * failure. Use inifile_get_error() to get last error.
 */
int inifile_init(struct inifile *, const char *conf);

/*
 * Return next parsed ini-file entry or NULL if last call 
 * failed (including end of file). Use inifile_get_error() 
 * to check the error condition.
 */
const struct inient * inifile_parse(struct inifile *);

/*
 * Release memory allocated by the parser.
 */
void inifile_free(struct inifile *);

/*
 * Set parser option. Returns 0 if successful and -1 on error. Use 
 * inifile_get_error() to check the cause of failure.
 */
int inifile_set_option(struct inifile *, int option, const void *value);
	
/*
 * Get parser option. Returns 0 if successful and -1 on error. Use 
 * inifile_get_error() to check the cause of failure.
 */
int inifile_get_option(struct inifile *, int option, void *value);

/*
 * Get last error or NULL if no error occured.
 */
const struct inierr * inifile_get_error(struct inifile *);

/*
 * Clear error object.
 */
void inifile_clear_error(struct inifile *);
	
#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif  /* __LIBINIFILE_INIFILE_H__ */
