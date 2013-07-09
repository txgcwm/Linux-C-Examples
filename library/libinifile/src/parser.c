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

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_STDAFX_H
# include "stdafx.h"
#endif

#ifdef HAVE_GETLINE
# include <stdio.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "common.h"

#define PARSE_ERROR 1
#define PARSE_NEXT  2
#define PARSE_DONE  3

#define PUTSTR_CHUNK 64

/*
 * Append character ch to put buffer buff.
 */
static char * putstr(char *buff, char ch)
{
	size_t pos = 0;

	if(buff) {
		pos = strlen(buff);
	}

	if((pos % PUTSTR_CHUNK) == 0) {
		buff = realloc(buff, pos + PUTSTR_CHUNK + 1);
		if(!buff) {
			return NULL;
		}
	}
	
	buff[pos] = ch;
	buff[pos + 1] = '\0';
	
	return buff;
}

/*
 * Exception handler.
 */
static const parser_entry * parser_error(struct inifile *inf, token_data *data, const char *msg)
{
	inifile_set_error(inf, data->line, data->pos, "parse error: %s", msg);
	return NULL;
}

static int parser_tokenize(struct inifile *inf, token_data *data, parser_entry *entry)
{
#ifdef DEBUG
	static const char *strtoken[] = {
		"NONE",   "BSECT",   "ESECT", "COMMENT", 
		"ASSIGN", "WHITESP", "QUOTE", "CDATA", 
		"EOSTR",  "MLINE"
	};
	static const char *strclass[] = {
		"GLOBAL", "SECTION", "KEYWORD", "VALUE"
	};
#endif
	
	/*
	 * Tokenize input string.
	 */
	while(token_get(inf, data)) {
		
#ifdef DEBUG
		printf("debug: (%d:%d): char='%c', curr=%s, prev=%s, seen=%s, class=%s\n",
		       data->line, data->pos, 
		       isprint(inf->str[data->pos]) ? inf->str[data->pos] : ' ', 
		       strtoken[data->curr],
		       strtoken[data->prev],
		       strtoken[data->seen],
		       strclass[data->cls]);
#endif
		/*
		 * Validate input
		 */
		if(inf->options & INIFILE_CHECK_SYNTAX) {
			if(!lexer_check(inf, data)) {
				return PARSE_ERROR;
			}
		}
		
		switch(data->curr)
		{
		case BSECT:				/* Begin section */
			if(entry->sect) {
				free(entry->sect);
				entry->sect = NULL;
			}
			break;
		case ESECT:				/* End section */
			break;
		case COMMENT:			        /* We are done */
		case EOSTR:
			if(data->seen != ESECT && data->prev != MLINE) {
				/*
				 * Only returns entries where keyword is set.
				 */ 
				token_trim(entry);
				if(entry->key && strlen(entry->key)) {
					return PARSE_DONE;
				}
			}
			/*
			 * Jump to next line.
			 */
			return PARSE_NEXT;
			break;
		case CDATA:
		case WHITESP:
			if(data->curr == WHITESP && data->prev == MLINE) {
				if((inf->options & INIFILE_COMPACT_MLINE) == 0) {
					entry->val = putstr(entry->val, inf->str[data->pos]);
				}
				data->pos++;
				continue;        /* eat whitespace */
			}
			if(data->seen == BSECT) {
				data->cls = SECTION;
				entry->sect = putstr(entry->sect, inf->str[data->pos]);
			} else if(data->seen == ASSIGN ||
				  data->seen == QUOTE) {
				data->cls = VALUE;
				entry->val = putstr(entry->val, inf->str[data->pos]);
			} else {
				data->cls = KEYWORD;
				entry->key = putstr(entry->key, inf->str[data->pos]);
			}
			break;
		case ASSIGN:
			/*
			 * Allow assignment inside values.
			 */
			if(inf->options & INIFILE_ASSIGN_INSIDE) {
				if(data->seen == ASSIGN && data->cls == VALUE) {
					entry->val = putstr(entry->val, inf->str[data->pos]);
				}
			}
			break;
		case NONE:				/* Ignore */
		case QUOTE:				/* Ignore */
		case MLINE:
			break;
		}
		
		/*
		 * Save last seen token thats not CDATA.
		 */
		if(data->curr != CDATA && data->curr != WHITESP && data->curr != MLINE) {
			data->seen = data->curr;
		}
		data->prev = data->curr;
		
		/*
		 * Move to next char.
		 */
		data->pos++;
	}
	
	return PARSE_NEXT;
}

/*
 * Parse next entry from ini-file.
 */
const parser_entry * parser_get_next(struct inifile *inf)
{
	token_data data;
	memset(&data, 0, sizeof(token_data));
	
	/*
	 * Label where we continue if end of string or
	 * multiline token is found in input stream.
	 */
	next:
	
	if(data.prev != MLINE) {								
		/*
		 * Reset scan data.
		 */
		if(inf->entry->key) {
			free(inf->entry->key);
			inf->entry->key = NULL;
		}
		if(inf->entry->val) {
			free(inf->entry->val);
			inf->entry->val = NULL;
		}
	}
	
	/*
	 * Get next line from input stream.
	 */
	while((inf->len = getline(&(inf->str), &(inf->size), inf->fs)) != -1) {
		
		/*
		 * Count up one more line.
		 */
		inf->entry->line++;
			
		/*
		 * Skip empty and commented lines.
		 */
		if(inf->str[0] == '\n' || strchr(inf->comment, inf->str[0])) {
			continue;
		}
		if(inf->str[inf->len - 1] == '\n') {
			inf->str[inf->len - 1] = '\0';
		}
		
		/*
		 * Fill line scanning data object.
		 */
		data.pos  = 0;
		data.line = inf->entry->line;
		if(data.prev != MLINE) {
			data.curr = NONE;
			data.prev = NONE;
			data.seen = NONE;
			data.cls  = GLOBAL;
			memset(&data.quote, 0, sizeof(token_quote));
		}
		
		switch(parser_tokenize(inf, &data, inf->entry)) {
		case PARSE_ERROR:
			return NULL;
		case PARSE_NEXT:
			goto next;
		case PARSE_DONE:
			if(inf->options & INIFILE_CHECK_SYNTAX) {
				/* 
				 * Handle syntax error that would require more context (i.e. look-ahead) 
				 * than whats available to the lexer when its doing its syntax check.
				 */
				if(data.seen == ASSIGN && (inf->entry->val == NULL || strlen(inf->entry->val) == 0)) {
					return parser_error(inf, &data, "assign without value");
				}
			}
			return inf->entry;
		}
			
	}
		
	/*
	 * Nothing found.
	 */
	return NULL;
}
