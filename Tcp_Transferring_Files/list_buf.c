#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "list_buf.h"

void list_buf_init(struct list_head *head)
{
	INIT_LIST_HEAD(head);
}

void del_data_from_list(struct h264_raw *entry)
{
	list_del(&entry->list);
	free(entry->data);
	free(entry);
	
	return;
}

void add_data_to_list(unsigned char *data, int len, struct list_head *head)
{
	struct h264_raw *entry = NULL;

	entry = (struct h264_raw *)malloc(sizeof(struct h264_raw));
	if(entry == NULL) {
		printf("have no memory\n");
		return;
	}

	memset(entry, 0, sizeof(struct h264_raw));
	entry->data = (unsigned char *)malloc(len);
	if(entry->data == NULL) {
		printf("have no memory\n");
		free(entry);
		return;
	}

	memset(entry->data, 0, len);
	memcpy(entry->data, data, len);
	entry->size = len;
	entry->position = 0;
	list_add_tail(&entry->list, head);

	return;
}

void read_data(struct h264_raw *entry, int len, int fd)
{
	int size = 0;
	ssize_t write_len = 0;

	if((size = entry->size - entry->position) >= len)
		size = len;
	
	if((write_len = write(fd, entry->data+entry->position, size)) >= 0)
		entry->position += write_len;

	if(entry->position == entry->size)
		del_data_from_list(entry);

	return;
}
