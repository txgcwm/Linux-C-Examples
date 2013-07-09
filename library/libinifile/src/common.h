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

/* 
 * The entries parsed should be on standard KEY = VAL form with '#' defining 
 * the beginning of a comment. Optional sections [SECT] may be used.
 */

#ifndef __LIBINIFILE_COMMON_H__
#define __LIBINIFILE_COMMON_H__

#include <sys/types.h>

#include "inifile.h"

/*************************************
 * The config parser. 
 *************************************/

/*
 * Creates alias for inient.
 */
#define parser_entry struct inient

/*
 * Parse next entry.
 */
const parser_entry * parser_get_next(struct inifile *);

/*************************************
 * The tokenizer.
 *************************************/

/*
 * Charaters recognized.
 */
enum TOKEN_SYMBOL
{
	NONE,		/* Dummy */
	BSECT,		/* Section begin ('[') */
	ESECT,		/* Section ends  (']') */
	COMMENT,	/* Comment begin ('#') */
	ASSIGN,		/* Keword assign ('=') */
	WHITESP,	/* Any whitespace character */
	QUOTE,		/* Quote character ('"' or ''') */
	CDATA,		/* Data */
	EOSTR,	        /* End of string */
	MLINE,          /* Multiline separator ('\') */	
};

/*
 * Character data classification.
 */
enum TOKEN_CLASSIFY
{
	GLOBAL, SECTION, KEYWORD, VALUE
};

typedef struct
{
	char sch;
	char ech;
	unsigned int num;
} token_quote;
		
typedef struct
{
	ssize_t pos;	          /* Start position */
	ssize_t line;	          /* Line number */
	int curr;		  /* Current token */
	int prev;                 /* Previous token */
	int seen;		  /* Saved token */
	int cls;                  /* Classification */
	token_quote quote;	  /* Quote string */
} token_data;

/* 
 * Get next TOKEN_SYMBOL 
 */
int token_get(struct inifile *, token_data *data);

/*
 * Remove leading and trailing whitespace.
 */
parser_entry * token_trim(parser_entry *entry);

/*************************************
 * The lexer.
 *************************************/

/*
 * Check lexical syntax.
 */
token_data * lexer_check(struct inifile *, token_data *data);

/*
 * Write error message.
 */
void inifile_set_error(struct inifile *, 
		       unsigned int line, 
		       unsigned int pos,
		       const char *fmt, ...);

/*
 * Remove leading and trailing whitespace characters from
 * string pointed to by str.
 */
char * inifile_trim_str(char *str);

#if !defined(HAVE_GETLINE) || !defined(HAVE_GETDELIM)
ssize_t rpl_getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
# if !defined(HAVE_GETDELIM)
#  define getdelim(p, n, d, s) rpl_getdelim((p), (n), (d), (s))
# endif
# if !defined(HAVE_GETLINE)
#  define getline(p, n, s) rpl_getdelim((p), (n), '\n', (s))
# endif
#endif  /* !defined(HAVE_GETLINE) || !defined(HAVE_GETDELIM) */

#endif  /* __LIBINIFILE_COMMON_H__ */
