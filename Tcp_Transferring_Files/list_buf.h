#ifndef _LIST_BUF_H_
#define _LIST_BUF_H_

#include "list.h"

struct h264_raw {
	unsigned char *data;
	int size;
	int position;
	struct list_head list;
};

void list_buf_init(struct list_head *head);

void del_data_from_list(struct h264_raw *entry);

void add_data_to_list(unsigned char *data, int len, struct list_head *head);

void read_data(struct h264_raw *entry, int len, int fd);

#endif