// ***********************************************************************
// libinifile - library for parsing ini-style configuration files.
// Copyright (C) 2008  Anders Lövgren
// ***********************************************************************
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// ***********************************************************************
// Send questions to: Anders Lövgren <lespaul@algonet.se>
// ***********************************************************************

// 
// This is simply a wrapper around the C library functions.
// 

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstdlib>

#include "inifile.h"
#include "inifile++.hpp"

#ifdef __cplusplus

// 
// Copies error message and clear library error.
// 
inifilepp::parser::exception::exception(inifile *inf, const inierr *err)
	: line(err->line), cpos(err->pos)
{
	msg = err->msg;
	inifile_clear_error(inf);
}

// 
// Initilize the library "handle" object. Throw exception on
// library call failure.
// 
inifilepp::parser::parser(const char *path)
{
	inf = new inifile;
	if(inifile_init(inf, path) < 0) {
		throw exception(inf, inifile_get_error(inf));
	}
}

// 
// Free resources associated with library "handle" and
// delete the handle itself.
// 
inifilepp::parser::~parser()
{
	inifile_free(inf);
	delete inf;
}

const inifilepp::parser::entry * inifilepp::parser::next() const
{
	return reinterpret_cast<const entry *>(inifile_parse(inf));
}

// 
// Get value of option (for int options). We let the C library
// handle all boundary checks for option argument.
// 
void inifilepp::parser::getopt(int option, int &value)
{
	if(inifile_get_option(inf, option, &value) < 0) {
		throw exception(inf, inifile_get_error(inf));
	}
}

// 
// Get value of option (for char ** options). We let the C library
// handle all boundary checks for option argument.
// 
void inifilepp::parser::getopt(int option, char **value)
{
	if(inifile_get_option(inf, option, value) < 0) {
		throw exception(inf, inifile_get_error(inf));
	}
}

// 
// Set value of option (for int options). We let the C library
// handle all boundary checks for option argument.
// 
void inifilepp::parser::setopt(int option, int value)
{
	if(inifile_set_option(inf, option, &value) < 0) {
		throw exception(inf, inifile_get_error(inf));
	}
}

// 
// Set value of option (for char * options). We let the C library
// handle all boundary checks for option argument.
// 
void inifilepp::parser::setopt(int option, const char *value)
{
	if(inifile_set_option(inf, option, value) < 0) {
		throw exception(inf, inifile_get_error(inf));
	}
}

#endif // __cplusplus
