#include  <sys/stat.h>
#include  <stdlib.h>
#include  <strings.h>			// for strncasecmp

#include  "IniFile.h"


IniFileData_TS f_inifile;

/*******************************************************************************
 *   desc: initialize member vars
 *------------------------------------------------------------------------------
 *  param: none
 *------------------------------------------------------------------------------
 * return: TRUE   -- ok
*******************************************************************************/
int IniInit(void)
{
	f_inifile.p_inifile = NULL;
	f_inifile.i_filesize = 0;

	memset(f_inifile.sz_filebuffer, 0, sizeof(f_inifile.sz_filebuffer));
	f_inifile.b_bufferchanged = FALSE;

	memset(f_inifile.sz_lastsection, 0, sizeof(f_inifile.sz_lastsection));
	f_inifile.b_sectionfound = FALSE;
	f_inifile.i_sc_startpos = 0;
	f_inifile.i_sc_endpos = 0;

	f_inifile.i_value_startpos = 0;
	f_inifile.i_value_endpos = 0;

	return TRUE;
}

/*******************************************************************************
 *   desc: open ini file
 *------------------------------------------------------------------------------
 *  param: char *psz_file       -- file to open
 *------------------------------------------------------------------------------
 * return:  0   -- file succefully opened
 *         -1   -- fail to open ini file
 *         -2   -- fail to read file to buffer
*******************************************************************************/
int IniOpenFile(const char *psz_file)
{
	struct stat statbuf;

	IniCloseFile();
	IniInit();

	stat(psz_file, &statbuf);
	DBGPRINT(("OpenFile -- ini file size = %ld\n", statbuf.st_size));

	// open file
	f_inifile.p_inifile = fopen(psz_file, "r+");
	if (f_inifile.p_inifile == NULL)
		return -1;

	// read file to buffer
	if (fread(f_inifile.sz_filebuffer, statbuf.st_size, 1, f_inifile.p_inifile)
		!= 1) {
		if (f_inifile.p_inifile != NULL)
			fclose(f_inifile.p_inifile);
		f_inifile.p_inifile = NULL;
		return -2;
	}
	rewind(f_inifile.p_inifile);
	f_inifile.i_filesize = statbuf.st_size;	// save file size

	return 0;
}

/*******************************************************************************
 *   desc: close ini file
 *------------------------------------------------------------------------------
 *  param: none
 *------------------------------------------------------------------------------
 * return:  0   -- file succefully closed
 *         -1   -- fail to close the opened file
*******************************************************************************/
int IniCloseFile(void)
{
	// file not opened
	if (f_inifile.p_inifile == NULL)
		return TRUE;

	// save file if buffer changed
	if (f_inifile.b_bufferchanged) {
		DBGPRINT(("\nCloseFile -- wrtie file, file size = %ld\n",
				  f_inifile.i_filesize));
		//rewind( p_inifile );
		fwrite(f_inifile.sz_filebuffer, f_inifile.i_filesize, 1,
			   f_inifile.p_inifile);
		f_inifile.b_bufferchanged = FALSE;
	}
	// close file
	if (fclose(f_inifile.p_inifile) != -1) {
		f_inifile.p_inifile = NULL;
		return TRUE;
	} else {
		return FALSE;
	}
}

/*******************************************************************************
 *   desc: get a string value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         char *       psz_value       -- key value
 *------------------------------------------------------------------------------
 * return: TRUE   -- key value found
 *         FALSE  -- key value not found
*******************************************************************************/
int IniGetString(const char *psz_section, const char *psz_key, char *psz_value)
{
	long i = 0;
	long j = 0;
	int b_skip = FALSE;

	// key name can't be null
	if (strlen(psz_key) == 0)
		return FALSE;

	// return if section not found
	if (IniSearchSection(psz_section) == FALSE)
		return FALSE;

	DBGPRINT(("\nGetString -- to get value of %s\n", psz_key));
	i = f_inifile.i_sc_startpos;
	while (i < f_inifile.i_sc_endpos) {
		// skip space, tab and \n
		while (i < f_inifile.i_filesize && (f_inifile.sz_filebuffer[i] == ' ' ||
											f_inifile.sz_filebuffer[i] == '\t'
											|| f_inifile.sz_filebuffer[i] ==
											'\n'))
			i++;
		// return if reach end of section
		if (i >= f_inifile.i_sc_endpos)
			return FALSE;

		b_skip = FALSE;
		switch (f_inifile.sz_filebuffer[i]) {
		case '#':				// a comment line
			b_skip = TRUE;
			break;
		default:
			if (strncasecmp
				(f_inifile.sz_filebuffer + i, psz_key, strlen(psz_key)) == 0) {
				//======================================================================
				// key matched, try to find value
				//======================================================================
				i += strlen(psz_key);
				// 1. skip space and tab
				while (i < f_inifile.i_sc_endpos &&
					   (f_inifile.sz_filebuffer[i] == ' '
						|| f_inifile.sz_filebuffer[i] == '\t'))
					i++;
				if (i >= f_inifile.i_sc_endpos)
					return FALSE;

				// 2. try to find '='
				if (f_inifile.sz_filebuffer[i] == '=') {
					//====================================================================
					// '=' found, get kay value
					//====================================================================
					i++;
					// skip space and tab
					while (i < f_inifile.i_sc_endpos &&
						   (f_inifile.sz_filebuffer[i] == ' '
							|| f_inifile.sz_filebuffer[i] == '\t'))
						i++;
					if (i >= f_inifile.i_sc_endpos)
						return TRUE;

					// search the end of the key value
					j = i;
					while (j < f_inifile.i_sc_endpos
						   && f_inifile.sz_filebuffer[j] != '\n')
						j++;
					j--;
					while (f_inifile.sz_filebuffer[j] == ' '
						   || f_inifile.sz_filebuffer[j] == '\t')
						j--;

					// copy the key value
					strncpy(psz_value, f_inifile.sz_filebuffer + i, j - i + 1);
					*(psz_value + j - i + 1) = '\0';
					f_inifile.i_value_startpos = i;
					f_inifile.i_value_endpos = j;
					DBGPRINT(("GetString -- value of %s is: %s\n", psz_key,
							  psz_value));

					return TRUE;
				} else {
					// no matching '=', ignore the line
					b_skip = TRUE;
				}
			} else {
				// key not matched, ignore the line and forward
				b_skip = TRUE;
			}
			break;
		}

		if (b_skip) {
			// ignore the line and forward
			while (i < f_inifile.i_filesize
				   && f_inifile.sz_filebuffer[i] != '\n')
				i++;
			if (i >= f_inifile.i_filesize)
				return FALSE;
			i++;				// Jump to the next line
		}
	}

	return FALSE;
}

/*******************************************************************************
 *   desc: set a string value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         const char * psz_value       -- key value
 *------------------------------------------------------------------------------
 * return: TRUE   -- key value writen to buffer
*******************************************************************************/
int IniSetString(const char *psz_section, const char *psz_key,
				 const char *psz_value)
{
	char sz_value[M_MAX_VALUE_BUFFER_SIZE];
	int i_oldvaluelen = 0;		// lenght of old value
	int i_newvaluelen = 0;		// lenght of new value
	long i = 0;
	long i_temp = 0;

	//DBGPRINT(( "\nSetString -- to search section: %s\n", psz_section ));
	if (IniSearchSection(psz_section) == FALSE) {
		//==========================================================================
		// section not found, we append the section and key value
		// at the end of buffer
		//==========================================================================
		memset(sz_value, 0, sizeof(sz_value));
		sprintf(sz_value, "\n\n\n[%s]\n%s = %s\n\n\n", psz_section,
				psz_key, psz_value);
		i_temp = strlen(sz_value);
		strncpy(f_inifile.sz_filebuffer + f_inifile.i_filesize, sz_value,
				strlen(sz_value));
		f_inifile.i_filesize += i_temp;
		f_inifile.b_bufferchanged = TRUE;
		return TRUE;
	}

	if (IniGetString(psz_section, psz_key, sz_value)) {
		//==========================================================================
		// section and key found, replace value
		//==========================================================================
		i_oldvaluelen =
			f_inifile.i_value_endpos - f_inifile.i_value_startpos + 1;
		i_newvaluelen = strlen(psz_value);
		//DBGPRINT(( "SetString -- before update, file size = %d\n", i_filesize ));
		if (i_newvaluelen > i_oldvaluelen) {
			// new value is longer than old value ************************************
			// 1. get more space by moving content backward
			i_temp = i_newvaluelen - i_oldvaluelen;
			for (i = f_inifile.i_filesize - 1; i >= f_inifile.i_value_endpos;
				 i--) {
				f_inifile.sz_filebuffer[i + i_temp] =
					f_inifile.sz_filebuffer[i];
			}
			f_inifile.i_filesize += i_temp;
			f_inifile.sz_filebuffer[f_inifile.i_filesize + 1] = '\0';

			// 2. write new value to buffer
			strncpy(f_inifile.sz_filebuffer + f_inifile.i_value_startpos,
					psz_value, strlen(psz_value));
		} else if (i_newvaluelen < i_oldvaluelen) {
			// new value is shorter than old value ***********************************
			// 1. write new value to buffer
			strncpy(f_inifile.sz_filebuffer + f_inifile.i_value_startpos,
					psz_value, strlen(psz_value));
			// 2. replace the following chars with spaces
			i_temp = i_oldvaluelen - i_newvaluelen;
			for (i = 0; i < i_temp; i++) {
				f_inifile.sz_filebuffer[f_inifile.i_value_endpos - i] = ' ';
			}
		} else {
			// same length ***********************************************************
			// just replace old value
			strncpy(f_inifile.sz_filebuffer + f_inifile.i_value_startpos,
					psz_value, strlen(psz_value));
		}
		//DBGPRINT(( "SetString -- after  update, file size = %d\n", f_inifile.i_filesize ));
		f_inifile.b_bufferchanged = TRUE;
	} else {
		//==========================================================================
		// key not found, we add key value by inserting a new line
		//==========================================================================
		// 1. make new line
		memset(sz_value, 0, sizeof(sz_value));
		sprintf(sz_value, "\n%s = %s\n", psz_key, psz_value);
		i_temp = strlen(sz_value);

		// 2. move buffer for new line
		for (i = f_inifile.i_filesize; i >= f_inifile.i_sc_endpos; i--) {
			f_inifile.sz_filebuffer[i + i_temp] = f_inifile.sz_filebuffer[i];
		}

		// 3. copy new line to buffer
		strncpy(f_inifile.sz_filebuffer + f_inifile.i_sc_endpos, sz_value,
				strlen(sz_value));
		f_inifile.i_filesize += i_temp;
		f_inifile.b_bufferchanged = TRUE;
	}

	// search end position of content again
	IniSearchContentEnd(f_inifile.i_sc_startpos);

	return TRUE;
}

/*******************************************************************************
 *   desc: get a interger value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         int          i_default       -- default value
 *------------------------------------------------------------------------------
 * return: key value or default value
*******************************************************************************/
int IniGetInteger(const char *psz_section, const char *psz_key, int i_default)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	//memset( sz_buffer, 0, sizeof(sz_buffer) );
	if (IniGetString(psz_section, psz_key, sz_buffer)) {
		DBGPRINT(("GetInteger -- key value is: %s\n", sz_buffer));
		if (strlen(sz_buffer) > 2) {
			// maybe a hex value
			if (sz_buffer[0] == '0'
				&& (sz_buffer[1] == 'x' || sz_buffer[1] == 'X')) {
				return (int)(strtol(sz_buffer, (char **)NULL, 16));
			}
		}
		return atoi(sz_buffer);
	}

	return i_default;
}

/*******************************************************************************
 *   desc: set a interger value
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         const int    i_value         -- key value
 *------------------------------------------------------------------------------
 * return: TRUE
*******************************************************************************/
int IniSetInteger(const char *psz_section, const char *psz_key,
				  const int i_value)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	DBGPRINT(("SetInteger -- key value is: %d\n", i_value));
	memset(sz_buffer, 0, sizeof(sz_buffer));
	sprintf(sz_buffer, "%d", i_value);
	DBGPRINT(("SetInteger -- value buffer is: %s\n", sz_buffer));
	IniSetString(psz_section, psz_key, sz_buffer);

	return TRUE;
}

/*******************************************************************************
 *   desc: get a long value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         long         i_default       -- default value
 *------------------------------------------------------------------------------
 * return: key value or default value
*******************************************************************************/
long IniGetLong(const char *psz_section, const char *psz_key, long i_default)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	//memset( sz_buffer, 0, sizeof(sz_buffer) );
	if (IniGetString(psz_section, psz_key, sz_buffer)) {
		if (strlen(sz_buffer) > 2) {
			// maybe a hex value
			if (sz_buffer[0] == '0'
				&& (sz_buffer[1] == 'x' || sz_buffer[1] == 'X')) {
				return (strtol(sz_buffer, (char **)NULL, 16));
			}
		}
		return atol(sz_buffer);
	}

	return i_default;
}

/*******************************************************************************
 *   desc: set a long value
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         const long   i_value         -- key value
 *------------------------------------------------------------------------------
 * return: TRUE
*******************************************************************************/
int IniSetLong(const char *psz_section, const char *psz_key, const long i_value)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	memset(sz_buffer, 0, sizeof(sz_buffer));
	sprintf(sz_buffer, "%ld", i_value);
	IniSetString(psz_section, psz_key, sz_buffer);

	return TRUE;
}

/*******************************************************************************
 *   desc: get a double value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         double       i_default       -- default value
 *------------------------------------------------------------------------------
 * return: key value or default value
*******************************************************************************/
double IniGetDouble(const char *psz_section, const char *psz_key,
					double i_default)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	//memset( sz_buffer, 0, sizeof(sz_buffer) );
	if (IniGetString(psz_section, psz_key, sz_buffer)) {
		return atof(sz_buffer);
	}

	return i_default;
}

/*******************************************************************************
 *   desc: set a double value
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         const double i_value         -- key value
 *------------------------------------------------------------------------------
 * return: TRUE
*******************************************************************************/
int IniSetDouble(const char *psz_section, const char *psz_key,
				 const double i_value)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	memset(sz_buffer, 0, sizeof(sz_buffer));
	sprintf(sz_buffer, "%g", i_value);
	IniSetString(psz_section, psz_key, sz_buffer);

	return TRUE;
}

/*******************************************************************************
 *   desc: get a bool value by key
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         bool         b_default       -- default value
 *------------------------------------------------------------------------------
 * return: key value or default value
*******************************************************************************/
int IniGetBool(const char *psz_section, const char *psz_key, int b_default)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	//memset( sz_buffer, 0, sizeof(sz_buffer) );
	if (IniGetString(psz_section, psz_key, sz_buffer)) {
		DBGPRINT(("GetBool -- key value is: %s\n", sz_buffer));
		if (strncasecmp(sz_buffer, "y", strlen("y")) == 0 ||
			strncasecmp(sz_buffer, "yes", strlen("yes")) == 0 ||
			strncasecmp(sz_buffer, "true", strlen("true")) == 0)
			return TRUE;
		if (strncasecmp(sz_buffer, "n", strlen("n")) == 0 ||
			strncasecmp(sz_buffer, "no", strlen("no")) == 0 ||
			strncasecmp(sz_buffer, "false", strlen("false")) == 0)
			return FALSE;
	}

	return b_default;
}

/*******************************************************************************
 *   desc: set a bool value
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *         const char * psz_key         -- key name
 *         const bool   b_value         -- key value
 *------------------------------------------------------------------------------
 * return: TRUE
*******************************************************************************/
int IniSetBool(const char *psz_section, const char *psz_key, const int b_value)
{
	char sz_buffer[M_MAX_INTVAL_BUFFER_SIZE];

	memset(sz_buffer, 0, sizeof(sz_buffer));
	if (b_value)
		sprintf(sz_buffer, "%s", "true");
	else
		sprintf(sz_buffer, "%s", "false");
	IniSetString(psz_section, psz_key, sz_buffer);

	return TRUE;
}

/*******************************************************************************
 *   desc: search a section
 *------------------------------------------------------------------------------
 *  param: const char * psz_section     -- section name
 *------------------------------------------------------------------------------
 * return: TRUE   -- section found
 *         FALSE  -- section not found
*******************************************************************************/
int IniSearchSection(const char *psz_section)
{
	long i = 0;
	int b_skip = FALSE;

	f_inifile.b_sectionfound = FALSE;

	// file error
	if (f_inifile.p_inifile == NULL)
		return FALSE;

	// section name can't be null
	if (strlen(psz_section) == 0)
		return FALSE;

	// same section name to the previous one
	if (strncasecmp(f_inifile.sz_lastsection, psz_section, strlen(psz_section))
		== 0) {
		f_inifile.b_sectionfound = TRUE;
		return TRUE;
	}

	while (i < f_inifile.i_filesize) {
		// skip space, tab and \n
		while (i < f_inifile.i_filesize && (f_inifile.sz_filebuffer[i] == ' ' ||
											f_inifile.sz_filebuffer[i] == '\t'
											|| f_inifile.sz_filebuffer[i] ==
											'\n'))
			i++;
		// return if reach end of file
		if (i >= f_inifile.i_filesize)
			return FALSE;

		b_skip = FALSE;
		switch (f_inifile.sz_filebuffer[i]) {
		case '#':				// comment
			b_skip = TRUE;
			break;

		case '[':				// section begin mark -- [
			i++;
			while (i < f_inifile.i_filesize
				   && (f_inifile.sz_filebuffer[i] == ' '
					   || f_inifile.sz_filebuffer[i] == '\t'))
				i++;
			if (i >= f_inifile.i_filesize)
				return FALSE;

			if (strncasecmp
				(f_inifile.sz_filebuffer + i, psz_section,
				 strlen(psz_section)) == 0) {
				// found section name, we try to seek ']'
				i += strlen(psz_section);
				while (i < f_inifile.i_filesize
					   && (f_inifile.sz_filebuffer[i] == ' '
						   || f_inifile.sz_filebuffer[i] == '\t'))
					i++;
				if (i >= f_inifile.i_filesize)
					return FALSE;

				if (f_inifile.sz_filebuffer[i] == ']') {
					// ']' found, so we:

					// 1. save section name to sz_lastsection
					memset(f_inifile.sz_lastsection, 0,
						   sizeof(f_inifile.sz_lastsection));
					sprintf(f_inifile.sz_lastsection, "%s", psz_section);

					// 2. get start and end position of section content
					i++;
					IniSearchContentStart(i);
					IniSearchContentEnd(f_inifile.i_sc_startpos);
					DBGPRINT(("\nSearchSection -- section content start at %ld, end at %ld\n", f_inifile.i_sc_startpos, f_inifile.i_sc_endpos));
					f_inifile.b_sectionfound = TRUE;
					return TRUE;
				} else {
					// no matching ']'
					b_skip = TRUE;
				}
			} else {
				// section name not match
				b_skip = TRUE;
			}
			break;

		default:				// other
			b_skip = TRUE;
			break;
		}

		if (b_skip) {
			// ignore the line and forward
			while (i < f_inifile.i_filesize
				   && f_inifile.sz_filebuffer[i] != '\n')
				i++;
			if (i >= f_inifile.i_filesize)
				return FALSE;
			i++;				// Jump to the next line
		}
	}

	return FALSE;
}

/*******************************************************************************
 *   desc: search start position of section content
 *------------------------------------------------------------------------------
 *  param: const long i_position  -- next position to ']'<section name end mark>
 *------------------------------------------------------------------------------
 * return: TRUE     -- found
*******************************************************************************/
int IniSearchContentStart(const long i_position)
{
	long i = 0;
	long i_temp = 0;

	i = i_position;

	// we ignore the rest of sectio name line
	while (i < f_inifile.i_filesize && f_inifile.sz_filebuffer[i] != '\n')
		i++;
	if (f_inifile.sz_filebuffer[i] == '\n')
		i++;

	// if reach end of file, we append some \n
	if (i >= f_inifile.i_filesize) {
		for (i_temp = 0; i_temp < 2; i_temp++) {
			f_inifile.sz_filebuffer[i + i_temp] = '\n';
		}
		f_inifile.i_sc_startpos = i + 1;
		f_inifile.i_filesize += 2;
		f_inifile.b_bufferchanged = TRUE;
		return TRUE;
	}
	// not reach end of file
	f_inifile.i_sc_startpos = i;

	// if it's '['(which means no enough \n between setciotns), we insert some \n
	if (f_inifile.sz_filebuffer[i] == '[') {
		for (i_temp = f_inifile.i_filesize; i_temp >= f_inifile.i_sc_startpos;
			 i_temp--) {
			f_inifile.sz_filebuffer[i_temp + 3] =
				f_inifile.sz_filebuffer[i_temp];
		}
		for (i_temp = 0; i_temp < 3; i_temp++) {
			f_inifile.sz_filebuffer[f_inifile.i_sc_startpos + i_temp] = '\n';
		}
		f_inifile.b_bufferchanged = TRUE;
		f_inifile.i_filesize += 3;
	}

	return TRUE;
}

/*******************************************************************************
 *   desc: search end position of section content
 *------------------------------------------------------------------------------
 *  param: const long i_startpos      -- start position of section content
 *------------------------------------------------------------------------------
 * return: TRUE   -- found
 ******************************************************************************/
int IniSearchContentEnd(const long i_startpos)
{
	long i = 0;
	long i_temp = 0;

	i = i_startpos;

	// try to serach position of next '['
	while (i < f_inifile.i_filesize) {
		// skip space, tab and \n
		while (i < f_inifile.i_filesize && (f_inifile.sz_filebuffer[i] == ' ' ||
											f_inifile.sz_filebuffer[i] == '\t'
											|| f_inifile.sz_filebuffer[i] ==
											'\n'))
			i++;

		//==========================================================================
		// 1. found '[', we try to find a position before '['
		//==========================================================================
		if (f_inifile.sz_filebuffer[i] == '[') {
			//DBGPRINT(( "SearchContentEnd -- position of next [ = %ld\n", i ));
			// skip \n backword
			i_temp = i;
			while (i > i_startpos + 1 && f_inifile.sz_filebuffer[i - 1] == '\n')
				i--;
			f_inifile.i_sc_endpos = i;

			// if no enough \n between setciotns, we insert some \n
			if (f_inifile.i_sc_endpos == i_temp) {
				for (i = f_inifile.i_filesize; i >= i_temp; i--) {
					f_inifile.sz_filebuffer[i + 2] = f_inifile.sz_filebuffer[i];
				}
				for (i = 0; i < 2; i++) {
					f_inifile.sz_filebuffer[i_temp + i] = '\n';
				}
				f_inifile.i_filesize += 2;
				f_inifile.b_bufferchanged = TRUE;
			}
			return TRUE;
		} else {
			// ignore the line and forward
			while (i < f_inifile.i_filesize
				   && f_inifile.sz_filebuffer[i] != '\n')
				i++;
			if (f_inifile.sz_filebuffer[i] == '\n')
				i++;
		}

		//==========================================================================
		// 2. if reach end of file
		//==========================================================================
		if (i == f_inifile.i_filesize) {
			// skip \n backword
			while (i > i_startpos + 1 && f_inifile.sz_filebuffer[i - 1] == '\n')
				i--;
			f_inifile.i_sc_endpos = i;

			// we append some \n if not enough
			if (i >= f_inifile.i_filesize - 1) {
				i = f_inifile.i_filesize;
				for (i_temp = 0; i_temp < 2; i_temp++) {
					f_inifile.sz_filebuffer[i_temp + i] = '\n';
				}
				f_inifile.i_filesize += 2;
				f_inifile.b_bufferchanged = TRUE;
			}
			return TRUE;
		}
	}

	return TRUE;
}
