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

#include "common.h"

/*
 * Exception handler.
 */
static token_data * lexer_error(struct inifile *inf, token_data *data, const char *msg)
{
	inifile_set_error(inf, data->line, data->pos, "syntax error: %s", msg);
	return NULL;
}

/*
 * Validate syntax.
 */
token_data * lexer_check(struct inifile *inf, token_data *data)
{
	switch(data->curr) {
	case QUOTE:
		if((inf->options & INIFILE_ALLOW_QUOTE) == 0) {
			return lexer_error(inf, data, "quoted strings is not allowed");
		}
		if(data->seen == QUOTE) {
			if(data->quote.sch != data->quote.ech) {
				return lexer_error(inf, data, "unbalanced quoted string");
			}
		}
		break;
	case WHITESP:
	case CDATA:
	case NONE:
		break;
	case BSECT:
		if(data->seen != NONE) {
			return lexer_error(inf, data, "begin section inside section");
		}
		break;
	case ESECT:
		if(data->seen != BSECT) {
			return lexer_error(inf, data, "end section without begin section");
		}
		break;
	case ASSIGN:
		if(data->seen != NONE) {
			if(data->seen == BSECT) {
				return lexer_error(inf, data, "assignment inide section");
			} else if(data->seen == ASSIGN) {
				if(data->cls == VALUE && inf->options & INIFILE_ASSIGN_INSIDE) {
					/* 
					 * Allow assignment inside value.
					 */
					return data;
				}
				return lexer_error(inf, data, "dual assignment detected (misstype?)");
			} else if(data->seen == ESECT) {
				return lexer_error(inf, data, "assignment to section");
			} else {
				return lexer_error(inf, data, "assignment without keyword");
			}
		}
		if(data->cls != KEYWORD) {
			return lexer_error(inf, data, "assignment to non-keyword");
		}
		break;
	case EOSTR:
		if(data->seen != ASSIGN &&
		   data->seen != ESECT &&
		   data->seen != NONE &&
		   data->seen != QUOTE) {
			if(data->seen == BSECT) {
				return lexer_error(inf, data, "end of string while looking for matching end of section");
			} else {
				return lexer_error(inf, data, "unexpected end of string");
			}
		}
		if(data->seen == QUOTE) {
			if(!data->quote.ech) {
				if(data->prev != MLINE) {
					return lexer_error(inf, data, "unterminated quote");
				}
			}
			if(data->quote.num % 2) {
				return lexer_error(inf, data, "unbalanced number of quotes");
			}
		}
		break;
	case COMMENT:
		if(data->seen == ASSIGN ||
		   data->seen == BSECT) {
			if(data->cls != VALUE) {
				return lexer_error(inf, data, "assignment or section without value");
			}
		}
		break;
	}

	switch(data->prev) {
	case MLINE:
		if(data->curr == EOSTR || data->curr == WHITESP) {
			if((inf->options & INIFILE_ALLOW_MULTILINE) == 0) {
				return lexer_error(inf, data, "multiline value");
			}
			if(data->cls == KEYWORD) {
				return lexer_error(inf, data, "multiline keyword");
			}
		}
		break;
	}
	
	return data;
}
