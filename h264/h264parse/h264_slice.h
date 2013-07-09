#ifndef H264_SLICE_H
#define H264_SLICE_H

#include "h264_stream.h"
#include "h264_sequence_parameter_set.h"
#include "h264_pic_parameter_set.h"

typedef struct h264_slice_header_t {
    int first_mb_in_slice;
    int slice_type;
    int pic_parameter_set_id;
    int frame_num;
    int field_pic_flag;
    int bottom_field_flag;
    int idr_pic_id;
    int pic_order_cnt_lsb;
    int delta_pic_order_cnt_bottom;
    int delta_pic_order_cnt[2];
    int redundant_pic_cnt;
    int direct_spatial_mv_pred_flag;
    int num_ref_idx_active_override_flag;
    int num_ref_idx_l0_active_minus1;
    int num_ref_idx_l1_active_minus1;
    // h264_ref_pic_list_reordering ref_pic_list_reordering;
    // h264_pred_weight_table pred_weight_table;
    // h264_dec_ref_pic_marking dec_ref_pic_marking;
    int cabac_init_idc;
    int slice_qp_delta;
    int sp_for_switch_flag;
    int slice_qs_delta;
    int disable_deblocking_filter_idc;
    int slice_alpha_c0_offset_div2;
    int slice_beta_offset_div2;
    int slice_group_change_cycle;
} h264_slice_header_t;

typedef struct h264_slice_data_t {
    int mb_skip_run;
    int mb_skip_flag;
    int mb_field_decoding_flag;
    // h264_macroblock_layer_t macroblock_layer;
    int end_of_slice_flag;
} h264_slice_data_t;

typedef struct h264_slice_t {
    h264_slice_header_t *header;
    h264_slice_data_t *data;
} h264_slice_rbsp_t;

h264_slice_header_t *h264_slice_header(h264_stream_t *s, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps);
void h264_print_slice_header(h264_slice_header_t *sh, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps);
h264_slice_rbsp_t *h264_slice_layer_without_partitioning_rbsp(h264_stream_t *s, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps);
void h264_print_slice_layer_without_partitioning_rbsp(h264_slice_rbsp_t *sl, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps);

#endif