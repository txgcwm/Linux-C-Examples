#include <stdio.h>
#include <stdlib.h>

#include "h264_sequence_parameter_set.h"
#include "h264_parse.h"
#include "h264_vui_parameters.h"

h264_sequence_parameter_set_t *h264_sequence_parameter_set(h264_stream_t *s)
{
    int i;
    h264_sequence_parameter_set_t *sps = malloc(sizeof(h264_sequence_parameter_set_t));
    sps->profile_idc = h264_u(s, 8);
    sps->constraint_set0_flag = h264_u(s, 1);
    sps->constraint_set1_flag = h264_u(s, 1);
    sps->constraint_set2_flag = h264_u(s, 1);
    h264_u(s, 5); // reserved_zero_5bits
    sps->level_idc = h264_u(s, 8);
    sps->seq_parameter_set_id = h264_ue(s);
    sps->log2_max_frame_num_minus4 = h264_ue(s);
    sps->pic_order_cnt_type = h264_ue(s);
    if (sps->pic_order_cnt_type == 0) {
        sps->log2_max_pic_order_cnt_lsb_minus4 = h264_ue(s);
    } else if (sps->pic_order_cnt_type == 1) {
        sps->delta_pic_order_always_zero_flag = h264_se(s);
        sps->offset_for_non_ref_pic = h264_se(s);
        sps->offset_for_top_to_bottom_field = h264_se(s);
        sps->num_ref_frames_in_pic_order_cnt_cycle = h264_se(s);
        for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; ++i) {
            sps->offset_for_ref_frame[i] = h264_se(s);
        }
    }
    sps->num_ref_frames = h264_ue(s);
    sps->gaps_in_frame_num_value_allowed_flag = h264_u(s, 1);
    sps->pic_width_in_mbs_minus1 = h264_ue(s);
    sps->pic_height_in_map_units_minus1 = h264_ue(s);
    sps->frame_mbs_only_flag = h264_u(s, 1);
    if (!sps->frame_mbs_only_flag) {
        sps->mb_adaptive_frame_field_flag = h264_u(s, 1);
    }
    sps->direct_8x8_inference_flag = h264_u(s, 1);
    sps->frame_cropping_flag = h264_u(s, 1);
    if (sps->frame_cropping_flag) {
        sps->frame_crop_left_offset = h264_ue(s);
        sps->frame_crop_right_offset = h264_ue(s);
        sps->frame_crop_top_offset = h264_ue(s);
        sps->frame_crop_bottom_offset = h264_ue(s);
    }
    sps->vui_parameters_present_flag = h264_u(s, 1);
    if (sps->vui_parameters_present_flag) {
        sps->vui_parameters = h264_vui_parameters(s);
    }
    h264_rbsp_trailing_bits(s);
    return sps;
}

void h264_print_sequence_parameter_set(h264_sequence_parameter_set_t *sps)
{
    int i;
    printf("sequence_parameter_set {\n");
    printf("    profile_idc: %d\n", sps->profile_idc);
    printf("    constraint_set0_flag: %d\n", sps->constraint_set0_flag);
    printf("    constraint_set1_flag: %d\n", sps->constraint_set1_flag);
    printf("    constraint_set2_flag: %d\n", sps->constraint_set1_flag);
    printf("    level_idc: %d\n", sps->level_idc);
    printf("    seq_parameter_set_id: %d\n", sps->seq_parameter_set_id);
    printf("    log2_max_frame_num_minus4: %d\n", sps->log2_max_frame_num_minus4);
    printf("    pic_order_cnt_type: %d\n", sps->pic_order_cnt_type);
    if (sps->pic_order_cnt_type == 0) {
        printf("    log2_max_pic_order_cnt_lsb_minus4: %d\n", sps->log2_max_pic_order_cnt_lsb_minus4);
    } else if (sps->pic_order_cnt_type == 1) {
        printf("    delta_pic_order_always_zero_flag: %d\n", sps->delta_pic_order_always_zero_flag);
        printf("    offset_for_non_ref_pic: %d\n", sps->offset_for_non_ref_pic);
        printf("    offset_for_top_to_bottom_field: %d\n", sps->offset_for_top_to_bottom_field);
        printf("    num_ref_frames_in_pic_order_cnt_cycle: %d\n", sps->num_ref_frames_in_pic_order_cnt_cycle);
        for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; ++i) {
            printf("    offset_for_ref_frame[%d]: %d\n", i, sps->offset_for_ref_frame[i]);
        }
    }
    printf("    num_ref_frames: %d\n", sps->num_ref_frames);
    printf("    gaps_in_frame_num_value_allowed_flag: %d\n", sps->gaps_in_frame_num_value_allowed_flag);
    printf("    pic_width_in_mbs_minus1: %d\n", sps->pic_width_in_mbs_minus1);
    printf("    pic_height_in_map_units_minus1: %d\n", sps->pic_height_in_map_units_minus1);
    printf("    frame_mbs_only_flag: %d\n", sps->frame_mbs_only_flag);
    printf("    mb_adaptive_frame_field_flag: %d\n", sps->mb_adaptive_frame_field_flag);
    printf("    direct_8x8_inference_flag: %d\n", sps->direct_8x8_inference_flag);
    printf("    frame_cropping_flag: %d\n", sps->frame_cropping_flag);
    if (sps->frame_cropping_flag) {
        printf("    frame_crop_left_offset: %d\n", sps->frame_crop_left_offset);
        printf("    frame_crop_right_offset: %d\n", sps->frame_crop_right_offset);
        printf("    frame_crop_top_offset: %d\n", sps->frame_crop_top_offset);
        printf("    frame_crop_bottom_offset: %d\n", sps->frame_crop_bottom_offset);
    }
    printf("    vui_parameters_present_flag: %d\n", sps->vui_parameters_present_flag);
    if (sps->vui_parameters_present_flag) {
        h264_print_vui_parameters(sps->vui_parameters);
    }
    printf("  }\n");
}