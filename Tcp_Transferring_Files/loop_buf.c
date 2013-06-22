#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "loop_buf.h"

int init_loop_buf(struct loop_buf *buf)
{
    buf->buffer = (unsigned char *)malloc(LOOP_BUF_LEN);
    if(buf->buffer == NULL) {
        printf("%s,%d  malloc error!\n", __FUNCTION__, __LINE__);
    }

    memset(buf->buffer, 0, LOOP_BUF_LEN);
    buf->in = buf->out = 0;
    pthread_mutex_init(&buf->mutex,NULL);

    return 0;
}

unsigned int put_loop_buf(struct loop_buf *buf, unsigned char *buffer, unsigned int len)
{
    unsigned int l;

    len = min(len, LOOP_BUF_LEN - buf->in + buf->out);

    /* first put the data starting from buf->in to buffer end */
    l = min(len, LOOP_BUF_LEN - (buf->in & (LOOP_BUF_LEN - 1)));
    memcpy(buf->buffer + (buf->in & (LOOP_BUF_LEN - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(buf->buffer, buffer + l, len - l);

    buf->in += len;

    return len;
}

unsigned int get_loop_buf(struct loop_buf *buf, unsigned char *buffer, unsigned int len)
{
    unsigned int l;

    len = min(len, buf->in - buf->out);

    /* first get the data from buf->out until the end of the buffer */
    l = min(len, LOOP_BUF_LEN - (buf->out & (LOOP_BUF_LEN - 1)));
    memcpy(buffer, buf->buffer + (buf->out & (LOOP_BUF_LEN - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, buf->buffer, len - l);

    buf->out += len;

    if (buf->out == buf->in)
        buf->out=buf->in = 0;

    return len;
}

void loop_buf_reset(struct loop_buf *buf)
{
    buf->in = buf->out = 0;
}

unsigned int loop_buf_len(struct loop_buf *buf)
{
    return buf->in - buf->out;
}

void release_loop_buf(struct loop_buf *buf)
{
    pthread_mutex_destroy(&buf->mutex);
    free(buf->buffer);
}

void lock_buf(struct loop_buf *buf)
{
    pthread_mutex_lock(&buf->mutex);
}

void unlock_buf(struct loop_buf *buf)
{
    pthread_mutex_unlock(&buf->mutex);
}
