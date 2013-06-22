#include <string.h>
#include <stdio.h>

#include "desktop-entry-util.h"
#include "list.h"

static void print_info(struct DesktopEntry *d)
{
	if(d->Name)
		printf("Name: %s\n",d->Name);
	if(d->GenericName) 
		printf("GenericName: %s\n",d->GenericName);
	if(d->TryExec)
		printf("TryExec: %s\n",d->TryExec);
	if(d->Exec)
		printf("Exec: %s\n",d->Exec);
	if(d->Path)
		printf("Path: %s\n",d->Path);
	if(d->Icon)
		printf("Icon: %s\n",d->Icon);
}

void desktop_list_add_tail(struct DesktopEntry *new, struct list_head *head)
{
	list_add_tail(&new->list, head);

	return;
}

void desktop_list_items(struct list_head *head)
{
	struct list_head *ptr;
	struct DesktopEntry *entry;

	list_for_each(ptr, head) {
		entry = list_entry(ptr, struct DesktopEntry, list);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		print_info(entry);
	}

	return;
}
