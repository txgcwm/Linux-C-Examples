#include <stdio.h>
#include <stdlib.h>

#include "h264_parse.h"
#include "h264_sequence_parameter_set.h"
#include "h264_pic_parameter_set.h"
#include "h264_slice.h"

void print_byte(unsigned char c) {
    char s[9];
    int i;
    for (i = 7; i >= 0; i--) {
        s[7 - i] = BITAT(c, i) ? '1' : '0';
    }
    s[8] = '\0';
    printf("%s = %d\n", s, c);
}

h264_nal_unit_t *h264_byte_stream_nal_unit(h264_stream_t *s)
{
    if (!h264_more_data_in_byte_stream(s)) {
        return NULL;
    }
    h264_nal_unit_t *nu;
    while (h264_next_bits(s, 24) != 0x000001 && 
        h264_next_bits(s, 36) != 0x00000001) {
        h264_f(s, 8, 0x00); // leading_zero_8bits
    }
    if (h264_next_bits(s, 24) != 0x000001) {
        h264_f(s, 8, 0x00); // zero_byte
    }
    if (h264_more_data_in_byte_stream(s)) {
        //h264_u(s, 24);
        h264_f(s, 24, 0x000001); // start_code_prefix_one_3bytes
        nu = h264_nal_unit(s);
    }
    while (h264_more_data_in_byte_stream(s) &&
        h264_next_bits(s, 24) != 0x000001 && 
        h264_next_bits(s, 36) != 0x00000001) {
        h264_f(s, 8, 0x00); // trailing_zero_8bits
    }
    return nu;
}

// Only latest sps and pps. Good enough for One-Seg
static h264_sequence_parameter_set_t *latest_sps = NULL;
static h264_pic_parameter_set_t *latest_pps = NULL;

h264_nal_unit_t *h264_nal_unit(h264_stream_t *s)
{
    h264_nal_unit_t *nu = malloc(sizeof(h264_nal_unit_t));
    h264_u(s, 1);
    nu->nal_ref_idc = h264_u(s, 2);
    nu->nal_unit_type = h264_u(s, 5);
    int NumBytesInRBSP = 0;
    int prev_bit_pos = s->bit_pos;
    int prev_byte_pos = s->byte_pos;
    while (h264_next_bits(s, 24) != 0x000001 && h264_more_data_in_byte_stream(s)) {
        if (h264_stream_bytes_remaining(s) > 2 && h264_next_bits(s, 24) == 0x000003) {
            h264_u(s, 24);
            NumBytesInRBSP += 2;
        } else {
            h264_u(s, 8);
            NumBytesInRBSP++;
        }
    }
    s->bit_pos = prev_bit_pos;
    s->byte_pos = prev_byte_pos;
    uint8_t *rbsp_buffer = (uint8_t *)malloc(NumBytesInRBSP);
    h264_stream_t *rbsp_s = h264_stream_new(rbsp_buffer, NumBytesInRBSP);
    int i;
    for (i = 0; i < NumBytesInRBSP; ++i) {
        rbsp_buffer[i] = h264_u(s, 8);
        if (h264_stream_bytes_remaining(s) > 2 && h264_next_bits(s, 24) == 0x000003) {
            rbsp_buffer[i++] = h264_u(s, 8);
            h264_f(s, 8, 0x03); // emulation_prevention_three_byte
        }
    }
    switch (nu->nal_unit_type) {
        case 1:
            nu->rbsp = h264_slice_layer_without_partitioning_rbsp(rbsp_s, nu->nal_unit_type, latest_sps, latest_pps);
        break;
        case 5:
            nu->rbsp = h264_slice_layer_without_partitioning_rbsp(rbsp_s, nu->nal_unit_type, latest_sps, latest_pps);
        break;
        case 7:
            latest_sps = h264_sequence_parameter_set(rbsp_s);
            nu->rbsp = latest_sps;
        break;
        case 8:
            latest_pps = h264_pic_parameter_set(rbsp_s);
            nu->rbsp = latest_pps;
        break;
        default:
            nu->rbsp = NULL;
        break;
    }
    return nu;
}

h264_nal_unit_t *h264_print_nal_unit(h264_nal_unit_t *nu)
{
    printf("NAL unit {\n");
    printf("  nal_ref_idc: %d\n", nu->nal_ref_idc);
    printf("  nal_unit_type: %d\n", nu->nal_unit_type);
    printf("  rbsp: ");
    switch (nu->nal_unit_type) {
        case 1:
            h264_print_slice_layer_without_partitioning_rbsp((h264_slice_rbsp_t *)nu->rbsp, nu->nal_unit_type, latest_sps, latest_pps);
        break;
        case 5:
            h264_print_slice_layer_without_partitioning_rbsp((h264_slice_rbsp_t *)nu->rbsp, nu->nal_unit_type, latest_sps, latest_pps);
        break;
        case 7:
            h264_print_sequence_parameter_set((h264_sequence_parameter_set_t *)nu->rbsp);
        break;
        case 8:
            h264_print_pic_parameter_set((h264_pic_parameter_set_t *)nu->rbsp);
        break;
        default:
            printf("\n");
        break;
    }
    printf("}\n");
    return nu;
}

int h264_more_data_in_byte_stream(h264_stream_t *s)
{
    return h264_stream_bytes_remaining(s) > 0;
}

uint32_t h264_next_bits(h264_stream_t *s, int n)
{
    if (n % 8 == 0) {
        return h264_stream_peek_bytes(s, n / 8);
    }
    return h264_stream_peek_bits(s, n);
}

uint32_t h264_u(h264_stream_t *s, uint32_t n)
{
    //if (n % 8 == 0) {
    //    return h264_stream_read_bytes(s, n / 8);
    //}
    return h264_stream_read_bits(s, n);
}

static uint8_t h264_exp_golomb_bits[256] = {
8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 
};

uint32_t h264_ue(h264_stream_t *s) {
    uint32_t bits, read;
    uint8_t coded;
    bits = 0;
    while (1) {
        if (h264_stream_bytes_remaining(s) < 1) {
            read = h264_stream_peek_bits(s, s->bit_pos) << (8 - s->bit_pos);
            break;
        } else {
            read = h264_stream_peek_bits(s, 8);
            if (bits > 16) {
                break;
            }
            if (read == 0) {
                h264_stream_read_bits(s, 8);
                bits += 8;
            } else {
                break;
            }
        }
    }
    coded = h264_exp_golomb_bits[read];
    h264_stream_read_bits(s, coded);
    bits += coded;
    return h264_stream_read_bits(s, bits + 1) - 1;
}

void h264_f(h264_stream_t *s, uint32_t n, uint32_t pattern) {
    uint32_t val = h264_u(s, n);
    if (val != pattern) {
        fprintf(stderr, "h264_f Error: fixed-pattern doesn't match. \nexpected: %x \nactual: %x \n", pattern, (unsigned int)val);
        exit(0);
    }
}

int32_t h264_se(h264_stream_t *s)
{
    uint32_t ret;
    ret = h264_ue(s);
    if (!(ret & 0x1)) {
        ret >>= 1;
        return (int32_t)(- ret);
    } 
    return (ret + 1) >> 1;
}

void h264_rbsp_trailing_bits(h264_stream_t *s) {
    h264_f(s, 1, 1);
    h264_f(s, s->bit_pos, 0);
}
