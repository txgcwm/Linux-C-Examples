#include <stdio.h>
#include "h264_parse.h"

int main (int argc, char const *argv[])
{
    h264_stream_t *file_s = h264_stream_from_file("f_200_baseline.264");int i = 0;
    while (h264_more_data_in_byte_stream(file_s)) {i++;
        printf("[%d]: ", i);
        h264_print_nal_unit(h264_byte_stream_nal_unit(file_s));
    }
    return 0;
}
