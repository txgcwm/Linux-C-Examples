#include <stdio.h>
#include <stdlib.h>

#include "h264_slice.h"
#include "h264_parse.h"

h264_slice_header_t *h264_slice_header(h264_stream_t *s, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps)
{
    h264_slice_header_t *sh = (h264_slice_header_t *)malloc(sizeof(h264_slice_header_t));
    sh->first_mb_in_slice = h264_ue(s);
    sh->slice_type = h264_ue(s);
    sh->pic_parameter_set_id = h264_ue(s);
    sh->frame_num = h264_u(s, sps->log2_max_frame_num_minus4 + 4);
    if (!sps->frame_mbs_only_flag) {
        sh->field_pic_flag = h264_u(s, 1);
        if (sh->field_pic_flag) {
            sh->bottom_field_flag = h264_u(s, 1);
        }
    }
    if (nal_unit_type == 5) {
        sh->idr_pic_id = h264_ue(s);
    }
    if (sps->pic_order_cnt_type == 0) {
        sh->pic_order_cnt_lsb = h264_u(s, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (pps->pic_order_present_flag && !sh->field_pic_flag) {
            sh->delta_pic_order_cnt_bottom = h264_se(s);
        }
    }
    if (sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag) {
        sh->delta_pic_order_cnt[0] = h264_se(s);
        if (pps->pic_order_present_flag && !sh->field_pic_flag) {
            sh->delta_pic_order_cnt[1] = h264_se(s);
        }
    }
    if (pps->redundant_pic_cnt_present_flag) {
        sh->redundant_pic_cnt = h264_ue(s);
    }
    if (sh->slice_type % 5 == 1) { // B slice
        sh->direct_spatial_mv_pred_flag = h264_u(s, 1);
    }
    if (sh->slice_type % 5 == 0 || // P slice
        sh->slice_type % 5 == 3 || // SP slice
        sh->slice_type % 5 == 1 ) { // B slice
        sh->num_ref_idx_active_override_flag = h264_u(s, 1);
        if (sh->num_ref_idx_active_override_flag) {
            sh->num_ref_idx_l0_active_minus1 = h264_ue(s);
            if (sh->slice_type % 5 == 1) { // B slice
                sh->num_ref_idx_l1_active_minus1 = h264_ue(s);
            }
        }
    }
    return sh;
}

void h264_print_slice_header(h264_slice_header_t *sh, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps)
{
    printf("slice_header {\n");
    printf("    first_mb_in_slice: %d\n", sh->first_mb_in_slice);
    printf("    slice_type: %d\n", sh->slice_type);
    printf("    pic_parameter_set_id: %d\n", sh->pic_parameter_set_id);
    printf("    frame_num: %d (%d bits)\n", sh->frame_num, sps->log2_max_frame_num_minus4 + 4);
    if (!sps->frame_mbs_only_flag) {
        printf("    field_pic_flag: %d\n", sh->field_pic_flag);
        if (sh->field_pic_flag) {
            printf("    bottom_field_flag: %d\n", sh->bottom_field_flag);
        }
    }
    if (nal_unit_type == 5) {
        printf("    idr_pic_id: %d\n", sh->idr_pic_id);
    }
    if (sps->pic_order_cnt_type == 0) {
        printf("    pic_order_cnt_lsb: %d\n", sh->pic_order_cnt_lsb);
        if (pps->pic_order_present_flag && !sh->field_pic_flag) {
            printf("    delta_pic_order_cnt_bottom: %d\n", sh->delta_pic_order_cnt_bottom);
        }
    }
    if (sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag) {
        printf("    delta_pic_order_cnt[0]: %d\n", sh->delta_pic_order_cnt[0]);
        if (pps->pic_order_present_flag && !sh->field_pic_flag) {
            printf("    delta_pic_order_cnt[1]: %d\n", sh->delta_pic_order_cnt[1]);
        }
    }
    if (pps->redundant_pic_cnt_present_flag) {
        printf("    redundant_pic_cnt: %d\n", sh->redundant_pic_cnt);
    }
    if (sh->slice_type % 5 == 1) { // B slice
        printf("    direct_spatial_mv_pred_flag: %d\n", sh->direct_spatial_mv_pred_flag);
    }
    if (sh->slice_type % 5 == 0 || // P slice
        sh->slice_type % 5 == 3 || // SP slice
        sh->slice_type % 5 == 1 ) { // B slice
        printf("    num_ref_idx_active_override_flag: %d\n", sh->num_ref_idx_active_override_flag);
        if (sh->num_ref_idx_active_override_flag) {
            printf("    num_ref_idx_l0_active_minus1: %d\n", sh->num_ref_idx_l0_active_minus1);
            if (sh->slice_type % 5 == 1) { // B slice
                printf("    num_ref_idx_l1_active_minus1: %d\n", sh->num_ref_idx_l1_active_minus1);
            }
        }
    }
    printf("  }\n");
}

h264_slice_rbsp_t *h264_slice_layer_without_partitioning_rbsp(h264_stream_t *s, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps)
{
    printf("%d\n", nal_unit_type);
    h264_slice_rbsp_t *sl = (h264_slice_rbsp_t *)malloc(sizeof(h264_slice_rbsp_t));
    sl->header = h264_slice_header(s, nal_unit_type, sps, pps);
    return sl;
}

void h264_print_slice_layer_without_partitioning_rbsp(h264_slice_rbsp_t *sl, uint8_t nal_unit_type, h264_sequence_parameter_set_t *sps, h264_pic_parameter_set_t *pps)
{
    h264_print_slice_header(sl->header, nal_unit_type, sps, pps);
}

