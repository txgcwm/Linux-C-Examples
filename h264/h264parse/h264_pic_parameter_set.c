#include <stdio.h>
#include <stdlib.h>

#include "h264_parse.h"
#include "h264_pic_parameter_set.h"

h264_pic_parameter_set_t *h264_pic_parameter_set(h264_stream_t *s)
{
    h264_pic_parameter_set_t *pps = malloc(sizeof(h264_pic_parameter_set_t));
    pps->pic_parameter_set_id = h264_ue(s);
    pps->seq_parameter_set_id = h264_ue(s);
    pps->entropy_coding_mode_flag = h264_u(s, 1);
    pps->pic_order_present_flag = h264_u(s, 1);
    pps->num_slice_groups_minus1 = h264_ue(s);
    if (pps->num_slice_groups_minus1 > 0) {
        pps->slice_group_map_type = h264_ue(s);
        if (pps->slice_group_map_type == 0) {
            pps->run_length_minus1 = h264_ue(s); // FIXME: Not Correct. Really an array.
        } else if (pps->slice_group_map_type == 2) {
            pps->top_left = h264_ue(s); // FIXME: Not Correct. Really an array.
            pps->bottom_right = h264_ue(s); // FIXME: Not Correct. Really an array.
        } else if (pps->slice_group_map_type == 3 || 
            pps->slice_group_map_type == 4 || 
            pps->slice_group_map_type == 5) {
            pps->slice_group_change_direction_flag = h264_u(s, 1);
            pps->slice_group_change_rate_minus1 = h264_ue(s);
        } else if (pps->slice_group_map_type == 6) {
            pps->pic_size_in_map_units_minus1 = h264_ue(s);
            pps->slice_group_id = h264_ue(s); // FIXME: Not Correct. Really an array.
        }
    }
    pps->num_ref_idx_l0_active_minus1 = h264_ue(s);
    pps->num_ref_idx_l1_active_minus1 = h264_ue(s);
    pps->weighted_pred_flag = h264_u(s, 1);
    pps->weighted_bipred_idc = h264_u(s, 2);
    pps->pic_init_qp_minus26 = h264_se(s);
    pps->pic_init_qs_minus26 = h264_se(s);
    pps->chroma_qp_index_offset = h264_se(s);
    pps->deblocking_filter_control_present_flag = h264_u(s, 1);
    pps->constrained_intra_pred_flag = h264_u(s, 1);
    pps->redundant_pic_cnt_present_flag = h264_u(s, 1);
    h264_rbsp_trailing_bits(s);
    return pps;
}

void h264_print_pic_parameter_set(h264_pic_parameter_set_t *pps)
{
    printf("pic_parameter_set {\n");
    printf("    pic_parameter_set_id: %d\n", pps->pic_parameter_set_id);
    printf("    seq_parameter_set_id: %d\n", pps->seq_parameter_set_id);
    printf("    entropy_coding_mode_flag: %d\n", pps->entropy_coding_mode_flag);
    printf("    pic_order_present_flag: %d\n", pps->pic_order_present_flag);
    printf("    num_slice_groups_minus1: %d\n", pps->num_slice_groups_minus1);
    // FIXME: Code for slice groups is missing here.
    printf("    num_ref_idx_l0_active_minus1: %d\n", pps->num_ref_idx_l0_active_minus1);
    printf("    num_ref_idx_l1_active_minus1: %d\n", pps->num_ref_idx_l1_active_minus1);
    printf("    weighted_pred_flag: %d\n", pps->weighted_pred_flag);
    printf("    weighted_bipred_idc: %d\n", pps->weighted_bipred_idc);
    printf("    pic_init_qp_minus26: %d\n", pps->pic_init_qp_minus26);
    printf("    pic_init_qs_minus26: %d\n", pps->pic_init_qs_minus26);
    printf("    chroma_qp_index_offset: %d\n", pps->chroma_qp_index_offset);
    printf("    deblocking_filter_control_present_flag: %d\n", pps->deblocking_filter_control_present_flag);
    printf("    constrained_intra_pred_flag: %d\n", pps->constrained_intra_pred_flag);
    printf("    redundant_pic_cnt_present_flag: %d\n", pps->redundant_pic_cnt_present_flag);
    printf("  }\n");
}
