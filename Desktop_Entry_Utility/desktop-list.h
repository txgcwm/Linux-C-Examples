#ifndef _DESKTOP_LIST_H
#define _DESKTOP_LIST_H

#include "desktop-entry-util.h"

void desktop_list_add_tail(struct DesktopEntry *new, struct list_head *head);

void desktop_list_items(struct list_head *head);

#endif
