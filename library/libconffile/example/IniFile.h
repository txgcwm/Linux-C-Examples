/*******************************************************************************
 * class for reading Ini File on Unix/Linux/Window
*******************************************************************************/

#ifndef __CINIFILE_H__
#define __CINIFILE_H__

#include 	<string.h>
#include	<stdio.h>			// for FILE

#define 	M_MAX_BUFFER_SIZE           6000
#define 	M_MAX_VALUE_BUFFER_SIZE     512
#define 	M_MAX_INTVAL_BUFFER_SIZE    32
#define 	M_MAX_SECTION_SIZE          32

#define FALSE	0
#define TRUE	1

#define DBGPRINT(m) printf m

typedef struct {
	FILE *p_inifile;
	long i_filesize;

	char sz_filebuffer[M_MAX_BUFFER_SIZE];
	int b_bufferchanged;		// if TRUE, save file when close

	char sz_lastsection[M_MAX_SECTION_SIZE];	// previous section name
	int b_sectionfound;
	long i_sc_startpos;			// start/end position of the
	long i_sc_endpos;			// following lines of section

	long i_value_startpos;		// start/end position of key
	long i_value_endpos;		// value
} IniFileData_TS;


int IniInit(void);

int IniOpenFile(const char *psz_file);
int IniCloseFile(void);

int IniGetString(const char *psz_section, const char *psz_key, char *psz_value);
int IniSetString(const char *psz_section, const char *psz_key,
				 const char *psz_value);

int IniGetInteger(const char *psz_section, const char *psz_key, int i_default);
int IniSetInteger(const char *psz_section, const char *psz_key,
				  const int i_value);

long IniGetLong(const char *psz_section, const char *psz_key, long i_default);
int IniSetLong(const char *psz_section, const char *psz_key,
			   const long i_value);

double IniGetDouble(const char *psz_section, const char *psz_key,
					double i_default);
int IniSetDouble(const char *psz_section, const char *psz_key,
				 const double i_value);

int IniGetBool(const char *psz_section, const char *psz_key, int b_default);
int IniSetBool(const char *psz_section, const char *psz_key, const int b_value);

int IniSearchSection(const char *psz_section);
int IniSearchContentStart(const long i_startpos);
int IniSearchContentEnd(const long i_startpos);

#endif // __CINIFILE_H__
