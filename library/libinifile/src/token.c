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

#ifdef HAVE_STDAFX_H
# include "stdafx.h"
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <ctype.h>

#include "common.h"

/*
 * Remove leading and trailing whitespace.
 */
parser_entry * token_trim(parser_entry *entry)
{
	if(entry->sect) {
		inifile_trim_str(entry->sect);
	}
	if(entry->key) {
		inifile_trim_str(entry->key);
	}
	if(entry->val) {
		inifile_trim_str(entry->val);
	}
	
	return entry;
}

/*
 * Returns next token from input data.
 */
int token_get(struct inifile *inf, token_data *data)
{	
	if(data->quote.sch) {
		/*
		 * Parsing quoted string.
		 */
		switch(inf->str[data->pos]) {
		case '"':
		case '\'':
			/*
			 * Matched end quote character.
			 */
			data->curr = QUOTE;
			data->quote.ech = inf->str[data->pos];
			data->quote.num++;
			break;
		case '\\':
			data->curr = MLINE;
			break;
		default:
			/*
			 * Treat all input as data inside quoted string.
			 */
			if(!strlen(inf->str) || data->pos == inf->len) {
				data->curr = EOSTR;
			} else if(isspace(inf->str[data->pos])) {
				data->curr = WHITESP;
			} else {
				data->curr = CDATA;
			}
			break;
		}
	} else {
		switch(inf->str[data->pos]) {
		case '[':
			data->curr = BSECT;
			break;
		case ']':
			data->curr = ESECT;
			break;
		case '"':
		case '\'':
			data->curr = QUOTE;
			data->quote.sch = inf->str[data->pos];
			data->quote.num++;
			break;
		case '\\':
			data->curr = MLINE;
			break;
		default:
			if(strlen(inf->str) == 0 || inf->str[data->pos] == 0 || data->pos == inf->len) {
				data->curr = EOSTR;
			} else if(isspace(inf->str[data->pos])) {
				data->curr = WHITESP;
			} else {
				/*
				 * Force at least one whitespace before comment to prevent legitime
				 * comment chars inside value to be treated as start of a comment. 
				 * For example smb.conf uses ';' to separate hosts (inside value), but
				 * also use ';' for comments.
				 */
				if(strchr(inf->comment, inf->str[data->pos]) && data->prev == WHITESP) {
					data->curr = COMMENT;
				} else if(strchr(inf->assign, inf->str[data->pos])) {
					data->curr = ASSIGN;
				} else {
					data->curr = CDATA;
				}
			}
			break;
		}
	}
	return data->curr;
}
