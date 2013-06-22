#ifndef _LOOP_BUF_H_
#define _LOOP_BUF_H_

#include <pthread.h>

#define LOOP_BUF_LEN 	(1024*1024)    //we need to set the buffer enough big

#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	(void) (&_x == &_y);   \
	_x < _y ? _x : _y; })


struct loop_buf {
    pthread_mutex_t mutex;
    unsigned char *buffer;
    unsigned int in;
    unsigned int out;
};

int init_loop_buf(struct loop_buf *buf);

void release_loop_buf(struct loop_buf *buf); //may be deprecated <<- pthread_mutex_lock

unsigned int put_loop_buf(struct loop_buf *buf, unsigned char *buffer, unsigned int len);

unsigned int get_loop_buf(struct loop_buf *buf, unsigned char *buffer, unsigned int len);

void lock_buf(struct loop_buf *buf);

void unlock_buf(struct loop_buf *buf);

void loop_buf_reset(struct loop_buf *buf);

unsigned int loop_buf_len(struct loop_buf *buf);

#endif
