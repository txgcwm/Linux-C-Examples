#ifndef H264_SEQUENCE_PARAMETER_SET
#define H264_SEQUENCE_PARAMETER_SET

#include "h264_vui_parameters.h"

typedef struct h264_sequence_parameter_set_t {
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int level_idc;
    int seq_parameter_set_id;
    int log2_max_frame_num_minus4;
    int pic_order_cnt_type;
    int log2_max_pic_order_cnt_lsb_minus4;
    int delta_pic_order_always_zero_flag;
    int offset_for_non_ref_pic;
    int offset_for_top_to_bottom_field;
    int num_ref_frames_in_pic_order_cnt_cycle;
    int offset_for_ref_frame[255];
    int num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;
    int direct_8x8_inference_flag;
    int frame_cropping_flag;
    int frame_crop_left_offset;
    int frame_crop_right_offset;
    int frame_crop_top_offset;
    int frame_crop_bottom_offset;
    int vui_parameters_present_flag;
    h264_vui_parameters_t *vui_parameters;
} h264_sequence_parameter_set_t;

h264_sequence_parameter_set_t *h264_sequence_parameter_set(h264_stream_t *s);
void h264_print_sequence_parameter_set(h264_sequence_parameter_set_t *sps);

#endif