#ifndef H264_PARSE_H
#define H264_PARSE_H

#include "h264_stream.h"
#include "h264_nal_unit.h"

h264_nal_unit_t *h264_byte_stream_nal_unit(h264_stream_t *s);
h264_nal_unit_t *h264_nal_unit(h264_stream_t *bs);
h264_nal_unit_t *h264_print_nal_unit(h264_nal_unit_t *nu);
int h264_more_data_in_byte_stream(h264_stream_t *s);
uint32_t h264_next_bits(h264_stream_t *s, int n);
uint32_t h264_u(h264_stream_t *s, uint32_t n);
uint32_t h264_ue(h264_stream_t *s);
void h264_f(h264_stream_t *s, uint32_t n, uint32_t pattern);
int32_t h264_se(h264_stream_t *s);
void h264_rbsp_trailing_bits(h264_stream_t *s);

#endif