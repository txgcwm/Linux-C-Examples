#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

#include <stdint.h>
#include <pthread.h>

#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	(void) (&_x == &_y);   \
	_x < _y ? _x : _y; })

typedef struct _ring_buff_ {
	uint8_t *startp;
	uint8_t *endp;
	uint8_t *rdp;
	uint8_t *wrp;
	uint8_t *rdp_pre;
	uint32_t buffer_size;
	uint32_t data_size;

	pthread_mutex_t mutex;

}ring_buff,*ring_buff_p;

#define SIZEOF_RING_BUFF (sizeof(struct _ring_buff_))


int8_t ringbuff_init(ring_buff_p *ringbuff, uint32_t size);

int8_t ringbuff_reset(ring_buff_p ringbuff);

int8_t ringbuff_close(ring_buff_p ringbuff);

uint32_t ringbuff_buffer_size(ring_buff_p ringbuff);

uint32_t ringbuff_data_size(ring_buff_p ringbuff);

uint32_t ringbuff_idle_buffer_size(ring_buff_p ringbuff);

uint32_t ringbuff_read(ring_buff_p ringbuff, uint8_t *buffer, uint32_t len);

uint32_t ringbuff_read_seek(ring_buff_p ringbuff, int32_t offset, int whence);

uint32_t ringbuff_write(ring_buff_p ringbuff, uint8_t *buffer, uint32_t len);

#endif
