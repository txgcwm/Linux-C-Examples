#include <stdio.h>
#include <stdlib.h>

#include "h264_vui_parameters.h"
#include "h264_parse.h"

h264_vui_parameters_t *h264_vui_parameters(h264_stream_t *s)
{
    h264_vui_parameters_t *vp = (h264_vui_parameters_t *)malloc(sizeof(h264_vui_parameters_t));
    vp->aspect_ratio_info_present_flag = h264_u(s, 1);
    if (vp->aspect_ratio_info_present_flag) {
        vp->aspect_ratio_idc = h264_u(s, 8);
        if (vp->aspect_ratio_idc == 255) { // Extended_SAR
            vp->sar_width = h264_u(s, 8);
            vp->sar_height = h264_u(s, 8);
        }
    }
    vp->overscan_info_present_flag = h264_u(s, 1);
    if (vp->overscan_info_present_flag) {
        vp->overscan_appropriate_flag = h264_u(s, 1);
    }
    vp->video_signal_type_present_flag = h264_u(s, 1);
    if (vp->video_signal_type_present_flag) {
        vp->video_format = h264_u(s, 3);
        vp->video_full_range_flag = h264_u(s, 1);
        vp->colour_description_present_flag = h264_u(s, 1);
        if (vp->colour_description_present_flag) {
            vp->colour_primaries = h264_u(s, 8);
            vp->transfer_characteristics = h264_u(s, 8);
            vp->matrix_coefficients = h264_u(s, 8);
        }
    }
    vp->chroma_loc_info_present_flag = h264_u(s, 1);
    if (vp->chroma_loc_info_present_flag) {
        vp->chroma_sample_loc_type_top_field = h264_ue(s);
        vp->chroma_sample_loc_type_bottom_field = h264_ue(s);
    }
    vp->nal_hrd_parameters_present_flag = h264_u(s, 1);
    //if (vp->nal_hrd_parameters_present_flag) {
    //    vp->hrd_parameters = h264_read_hrd_parameters(s);
    //}
    vp->vcl_hrd_parameters_present_flag  = h264_u(s, 1);
    //if (vp->nal_hrd_parameters_present_flag) {
    //    vp->hrd_parameters = h264_read_hrd_parameters(s);
    //}
    if (vp->nal_hrd_parameters_present_flag || vp->vcl_hrd_parameters_present_flag) {
        vp->low_delay_hrd_flag = h264_u(s, 1);
    }
    vp->pic_struct_present_flag = h264_u(s, 1);
    vp->bitstream_restriction_flag = h264_u(s, 1);
    if (vp->bitstream_restriction_flag) {
        vp->bitstream_restriction_flag = h264_u(s, 1);
        vp->motion_vectors_over_pic_boundaries_flag = h264_u(s, 1);
        vp->max_bytes_per_pic_denom = h264_ue(s);
        vp->max_bits_per_mb_denom = h264_ue(s);
        vp->log2_max_mv_length_horizontal = h264_ue(s);
        vp->log2_max_mv_length_vertical = h264_ue(s);
        vp->num_reorder_frames = h264_ue(s);
        vp->max_dec_frame_buffering = h264_ue(s);
    }
    return vp;
}

void h264_print_vui_parameters(h264_vui_parameters_t *vp)
{
    printf("vui_parameters {\n");
    printf("    aspect_ratio_info_present_flag: %d\n", vp->aspect_ratio_info_present_flag);
    if (vp->aspect_ratio_info_present_flag) {
        printf("    aspect_ratio_idc: %d\n", vp->aspect_ratio_idc);
        if (vp->aspect_ratio_idc == 255) { // Extended_SAR
            printf("    sar_width: %d\n", vp->sar_width);
            printf("    sar_height: %d\n", vp->sar_height);
        }
    }
    printf("    overscan_info_present_flag: %d\n", vp->overscan_info_present_flag);
    if (vp->overscan_info_present_flag) {
        printf("    overscan_appropriate_flag: %d\n", vp->overscan_appropriate_flag);
    }
    printf("    video_signal_type_present_flag: %d\n", vp->video_signal_type_present_flag);
    if (vp->video_signal_type_present_flag) {
        printf("    video_format: %d\n", vp->video_format);
        printf("    video_full_range_flag: %d\n", vp->video_full_range_flag);
        printf("    colour_description_present_flag: %d\n", vp->colour_description_present_flag);
        if (vp->colour_description_present_flag) {
            printf("    colour_primaries: %d\n", vp->colour_primaries);
            printf("    transfer_characteristics: %d\n", vp->transfer_characteristics);
            printf("    matrix_coefficients: %d\n", vp->matrix_coefficients);
        }
    }
    printf("    chroma_loc_info_present_flag: %d\n", vp->chroma_loc_info_present_flag);
    if (vp->chroma_loc_info_present_flag) {
        printf("    chroma_sample_loc_type_top_field: %d\n", vp->chroma_sample_loc_type_top_field);
        printf("    chroma_sample_loc_type_bottom_field: %d\n", vp->chroma_sample_loc_type_bottom_field);
    }
    printf("    nal_hrd_parameters_present_flag: %d\n", vp->nal_hrd_parameters_present_flag);
    //if (vp->nal_hrd_parameters_present_flag) {
    //    vp->hrd_parameters = h264_read_hrd_parameters(bs);
    //}
    printf("    vcl_hrd_parameters_present_flag : %d\n", vp->vcl_hrd_parameters_present_flag );
    //if (vp->nal_hrd_parameters_present_flag) {
    //    vp->hrd_parameters = h264_read_hrd_parameters(bs);
    //}
    if (vp->nal_hrd_parameters_present_flag || vp->vcl_hrd_parameters_present_flag) {
        printf("    low_delay_hrd_flag: %d\n", vp->low_delay_hrd_flag);
    }
    printf("    pic_struct_present_flag: %d\n", vp->pic_struct_present_flag);
    printf("    bitstream_restriction_flag: %d\n", vp->bitstream_restriction_flag);
    if (vp->bitstream_restriction_flag) {
        printf("    bitstream_restriction_flag: %d\n", vp->bitstream_restriction_flag);
        printf("    motion_vectors_over_pic_boundaries_flag: %d\n", vp->motion_vectors_over_pic_boundaries_flag);
        printf("    max_bytes_per_pic_denom: %d\n", vp->max_bytes_per_pic_denom);
        printf("    max_bits_per_mb_denom: %d\n", vp->max_bits_per_mb_denom);
        printf("    log2_max_mv_length_horizontal: %d\n", vp->log2_max_mv_length_horizontal);
        printf("    log2_max_mv_length_vertical: %d\n", vp->log2_max_mv_length_vertical);
        printf("    num_reorder_frames: %d\n", vp->num_reorder_frames);
        printf("    max_dec_frame_buffering: %d\n", vp->max_dec_frame_buffering);
    }
}