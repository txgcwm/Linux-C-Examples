#ifndef H264_PIC_PARAMETER_SET_H
#define H264_PIC_PARAMETER_SET_H

#include "h264_stream.h"

typedef struct h264_pic_parameter_set_t {
    int pic_parameter_set_id;
    int seq_parameter_set_id;
    int entropy_coding_mode_flag;
    int pic_order_present_flag;
    int num_slice_groups_minus1;
    int slice_group_map_type;
    int run_length_minus1;
    int top_left;
    int bottom_right;
    int slice_group_change_direction_flag;
    int slice_group_change_rate_minus1;
    int pic_size_in_map_units_minus1;
    int slice_group_id;
    int num_ref_idx_l0_active_minus1;
    int num_ref_idx_l1_active_minus1;
    int weighted_pred_flag;
    int weighted_bipred_idc;
    int pic_init_qp_minus26;
    int pic_init_qs_minus26;
    int chroma_qp_index_offset;
    int deblocking_filter_control_present_flag;
    int constrained_intra_pred_flag;
    int redundant_pic_cnt_present_flag;
} h264_pic_parameter_set_t;

h264_pic_parameter_set_t *h264_pic_parameter_set(h264_stream_t *s);
void h264_print_pic_parameter_set(h264_pic_parameter_set_t *sps);

#endif
