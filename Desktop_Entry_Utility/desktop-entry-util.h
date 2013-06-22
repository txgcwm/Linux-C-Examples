#ifndef _DESKTOP_ENTRY_UTIL_H
#define _DESKTOP_ENTRY_UTIL_H

#include "list.h"


struct DesktopEntry {
	char *Name;
	char *GenericName;
	char *TryExec;
	char *Exec;
	char *Path;
	char *Icon;
	char *_path;
	
	struct list_head list;
};

void desktop_read_file(struct DesktopEntry *d, const char *filename);

#endif
