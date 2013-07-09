#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "h264_stream.h"

h264_stream_t *h264_stream_new(uint8_t *data, int size)
{
    h264_stream_t *s = malloc(sizeof(h264_stream_t));
    s->data = data;
    s->size = size;
    s->bit_pos = 7;
    s->byte_pos = 0;
    return s;
}

h264_stream_t *h264_stream_from_file(char *path)
{
    FILE *fp;
    long file_size;
    uint8_t *buffer;
    size_t result;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }
    
    fseek(fp , 0 , SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    
    buffer = (uint8_t *)malloc(file_size);
    assert(buffer);
    
    result = fread(buffer, 1, file_size, fp);
    assert(result == file_size);
    
    fclose (fp);
    
    return h264_stream_new(buffer, file_size);
}

void h264_stream_free(h264_stream_t *s)
{
    free(s->data);
    s->size = 0;
    s->bit_pos = 7;
    s->byte_pos = 0;
}

uint32_t h264_stream_read_bits(h264_stream_t *s, uint32_t n)
{
    uint32_t ret = 0;
    
    if (n == 0) {
        return 0;
    }
    
    int i;
    for (i = 0; i < n; ++i) {
        if (h264_stream_bits_remaining(s) == 0) {
            ret <<= n - i - 1;
        }
        uint8_t b = s->data[s->byte_pos];
        if (n - i <= 32) {
            ret = ret << 1 | BITAT(b, s->bit_pos);
        }
        if (s->bit_pos == 0) {
            s->bit_pos = 7;
            s->byte_pos++;
        } else {
            s->bit_pos--;
        }
    }
    return ret;
}

uint32_t h264_stream_peek_bits(h264_stream_t *s, uint32_t n)
{
    int prev_bit_pos = s->bit_pos;
    int prev_byte_pos = s->byte_pos;
    uint32_t ret = h264_stream_read_bits(s, n);
    s->bit_pos = prev_bit_pos;
    s->byte_pos = prev_byte_pos;
    return ret;
}

uint32_t h264_stream_read_bytes(h264_stream_t *s, uint32_t n)
{
    uint32_t ret = 0;
    
    if (n == 0) {
        return 0;
    }
    
    int i;
    for (i = 0; i < n; ++i) {
        if (h264_stream_bytes_remaining(s) == 0) {
            ret <<= (n - i - 1) * 8;
            break;
        }
        if (n - i <= 4) {
            ret = ret << 8 | s->data[s->byte_pos];
        }
        s->byte_pos++;
    }
    
    return ret;
}

uint32_t h264_stream_peek_bytes(h264_stream_t *s, uint32_t n)
{
    int prev_byte_pos = s->byte_pos;
    uint32_t ret = h264_stream_read_bytes(s, n);
    s->byte_pos = prev_byte_pos;
    return ret;
}

int h264_stream_bits_remaining(h264_stream_t *s)
{
    return (s->size - s->byte_pos) * 8 + s->bit_pos;
}

int h264_stream_bytes_remaining(h264_stream_t *s)
{
    return s->size - s->byte_pos;
}
