#ifndef H264_NAL_UNIT_H
#define H264_NAL_UNIT_H

#include "util.h"

typedef struct h264_nal_unit_t {
    uint8_t nal_ref_idc;
    uint8_t nal_unit_type;
    void *rbsp;
} h264_nal_unit_t;

#endif