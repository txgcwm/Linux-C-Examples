#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "desktop-entry-util.h"

#define MALLOC_SIZE	1024

static int dynamic_fgets(char **line, FILE * fp)
{
	size_t size = 0;
	int length = 0;

	do {
		if (size-length < MALLOC_SIZE) {
			if ((*line = realloc(*line, size + MALLOC_SIZE)) == NULL) {
				printf("no memory\n");
				break;
			}
			size += MALLOC_SIZE;
		}

		if (fgets(*line + length, MALLOC_SIZE, fp) == NULL) {
			break;
		}

		length += strlen(*line + length);
	} while ((*line)[length - 1] != '\n');

	return length;
}

static int desktop_entry_check(char *line)
{
	static int entry_flag = 0;
	char *ptr = line;

	if(ptr[0] != '[') {
		return entry_flag;
	}

	while(isspace(*++ptr));

	if(strncmp(ptr, "Desktop Entry", strlen("Desktop Entry")) == 0) {
		entry_flag = 1;
		return entry_flag;
	}
	
	entry_flag = 0;

	return entry_flag;
}

void desktop_read_file(struct DesktopEntry *d, const char *filename)
{
	FILE * fp = NULL;
	char *line = NULL;
	char *sep = NULL;
	size_t size = 0;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		return;
	}

	while ((size = dynamic_fgets(&line, fp)) > 0) {
		line[size - 1] = '\0';

		if ((line[0] != '#') && desktop_entry_check(line) && (sep = strchr(line, '=')) != NULL) {
			*sep = '\0';
			sep++;
			
			if (strcmp(line, "Name") == 0)
				d->Name = strdup(sep);
			else if (strcmp(line, "GenericName") == 0)
				d->GenericName = strdup(sep);
			else if (strcmp(line, "TryExec") == 0)
				d->TryExec = strdup(sep);
			else if (strcmp(line, "Exec") == 0)
				d->Exec = strdup(sep);
			else if (strcmp(line, "Path") == 0)
				d->Path = strdup(sep);
			else if (strcmp(line, "Icon") == 0)
				d->Icon = strdup(sep);
		}

		free(line);
		line = sep = NULL;
	} 

	fclose(fp);
	fp = NULL;

	return;
}
