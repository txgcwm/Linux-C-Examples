#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ringbuff.h"



int8_t ringbuff_init(ring_buff_p *ringbuff, uint32_t size)
{
	ring_buff_p new = NULL;

	new = (ring_buff_p) malloc(SIZEOF_RING_BUFF);
	if (new == NULL) {
        fprintf(stderr,"### Unable to allocate ringbuff datastructure\n");
        return 1;
    }

	memset(new, 0, SIZEOF_RING_BUFF);

	new->startp = (uint8_t *) malloc(size);
	if(new->startp == NULL) {
		fprintf(stderr,"unable to allocate data buffer!\n");
		free(new);
		return -1;
	}

	new->rdp = new->wrp = new->rdp_pre = new->startp;
	new->endp = new->startp + size;
	new->buffer_size = size;
	new->data_size = 0;

	pthread_mutex_init(&new->mutex,NULL);

	*ringbuff = new;

	return 0;
}

int8_t ringbuff_reset(ring_buff_p ringbuff)
{
	ringbuff->rdp = ringbuff->wrp = ringbuff->rdp_pre = ringbuff->startp;
	ringbuff->data_size = 0;

	return 0;
}

int8_t ringbuff_close(ring_buff_p ringbuff)
{
	free(ringbuff->startp);
	pthread_mutex_destroy(&ringbuff->mutex);
	free(ringbuff);

	return 0;
}

uint32_t ringbuff_buffer_size(ring_buff_p ringbuff)
{
	return ringbuff->buffer_size;
}

uint32_t ringbuff_data_size(ring_buff_p ringbuff)
{
	return ringbuff->data_size;
}

uint32_t ringbuff_idle_buffer_size(ring_buff_p ringbuff)
{
	return (ringbuff->buffer_size - ringbuff->data_size);
}

uint32_t ringbuff_read(ring_buff_p ringbuff, uint8_t *buffer, uint32_t len)
{
	uint32_t l = 0;

	if(!ringbuff->data_size)
		return 0;

	pthread_mutex_lock(&ringbuff->mutex);

	if(ringbuff->wrp >= ringbuff->rdp)
		len = min(len, (ringbuff->wrp - ringbuff->rdp));
	else
		len = min(len, (ringbuff->buffer_size - (ringbuff->rdp - ringbuff->wrp)));

	l = min(len, (ringbuff->endp - ringbuff->rdp));

	memcpy(buffer, ringbuff->rdp, l);
	memcpy(buffer + l, ringbuff->startp, len - l);

	ringbuff->rdp_pre = ringbuff->rdp;
	if(l < len)
		ringbuff->rdp = ringbuff->rdp + len - ringbuff->buffer_size;
	else
		ringbuff->rdp += len;
	
	ringbuff->data_size -= len;

	pthread_mutex_unlock(&ringbuff->mutex);

	return len;
}

uint32_t ringbuff_read_seek(ring_buff_p ringbuff, int32_t offset, int whence)
{
	uint32_t l = 0;

	pthread_mutex_lock(&ringbuff->mutex);

	pthread_mutex_unlock(&ringbuff->mutex);

	return 0;
}

uint32_t ringbuff_write(ring_buff_p ringbuff, uint8_t *buffer, uint32_t len)
{
	uint32_t l = 0;

	if(ringbuff->data_size == ringbuff->buffer_size)
		return 0;

	pthread_mutex_lock(&ringbuff->mutex);

	if(ringbuff->wrp >= ringbuff->rdp)
		len = min(len, (ringbuff->buffer_size - (ringbuff->wrp - ringbuff->rdp)));
	else
		len = min(len, (ringbuff->rdp - ringbuff->wrp));

	l = min(len, (ringbuff->endp - ringbuff->wrp));

	memcpy(ringbuff->wrp, buffer, l);
	memcpy(ringbuff->startp, buffer + l, len -l);

	if(l < len)
		ringbuff->wrp = ringbuff->wrp + len - ringbuff->buffer_size;
	else
		ringbuff->wrp += len;

	ringbuff->data_size += len;

	pthread_mutex_unlock(&ringbuff->mutex);

	return len;
}



