#ifndef H264_STREAM_H
#define H264_STREAM_H

#include "util.h"

typedef struct h264_stream_t {
    uint8_t *data;
    uint32_t size;
    int bit_pos;
    int byte_pos;
} h264_stream_t;

h264_stream_t *h264_stream_new(uint8_t *data, int size);
h264_stream_t *h264_stream_from_file(char *path);
void h264_stream_free(h264_stream_t *s);
uint32_t h264_stream_read_bits(h264_stream_t *s, uint32_t n);
uint32_t h264_stream_peek_bits(h264_stream_t *s, uint32_t n);
uint32_t h264_stream_read_bytes(h264_stream_t *s, uint32_t n);
uint32_t h264_stream_peek_bytes(h264_stream_t *s, uint32_t n);
int h264_stream_bits_remaining(h264_stream_t *s);
int h264_stream_bytes_remaining(h264_stream_t *s);

#endif