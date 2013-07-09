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

#ifndef __LIBINIFILE_INIFILE_CXX_H__
#define __LIBINIFILE_INIFILE_CXX_H__

#ifdef __cplusplus

#include <string>
#include <inifile.h>

namespace inifilepp {
	
	class parser
	{
	private:
		inifile *inf;
	public:
		//
		// An ini-file entry.
		//
		struct entry
		{
			unsigned int line;      // line number.
			const char *sect;	// section name.
			const char *key;	// entry keyword.
			const char *val;	// entry value.
		};
		
		//
		// Parse exception.
		//
		struct exception
		{
			unsigned int line;
			unsigned int cpos;
			std::string msg;
			exception(inifile *, const inierr *);
		};
		
		//
		// option argument for getopt()/setopt():
		//
		enum {
			// 
			// int options:
			// 
			INIFPP_CHECK_SYNTAX    =   1,        // syntax check
			INIFPP_ALLOW_QUOTE     =   2,        // allow quoted strings
			INIFPP_ASSIGN_INSIDE   =   4,        // allow s1=s2 inside values
			INIFPP_ALLOW_MULTILINE =   8,        // allow multiline ('\')
			INIFPP_COMPACT_MLINE   =  16,        // eat whitespace in multiline
			// 
			// char * options:
			// 
			INIFPP_CHARS_COMMENT   = 256,        // get/set comment chars
			INIFPP_CHARS_ASSIGN    = 512         // get/set assignment chars
		};
		
		parser(const char *path);
		~parser();
		const entry * next() const;                  // throws parser::exception
		
		void getopt(int option, int &value);         // throws parser::exception
		void getopt(int option, char **value);       // throws parser::exception
		void setopt(int option, int value);          // throws parser::exception
		void setopt(int option, const char *value);  // throws parser::exception
	};
	
}      // namespace inifilepp

#endif // __cplusplus

#endif // __LIBINIFILE_INIFILE_CXX_H__
