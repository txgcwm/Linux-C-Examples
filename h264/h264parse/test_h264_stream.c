#include <stdlib.h>

#include "CuTest.h"
#include "h264_stream.h"
#include "test_h264_stream.h"

void test_h264_stream_read_bits(CuTest* tc)
{
    uint8_t data[3] = {0xae, 0xcf, 0x0b};
    h264_stream_t *s = h264_stream_new(data, 3);
    CuAssertIntEquals(tc, 1, h264_stream_read_bits(s, 1));
    CuAssertIntEquals(tc, 1, h264_stream_read_bits(s, 2));
    CuAssertIntEquals(tc, 14, h264_stream_read_bits(s, 5));
    CuAssertIntEquals(tc, 13250, h264_stream_read_bits(s, 14));
    CuAssertIntEquals(tc, 3, h264_stream_read_bits(s, 2));
}

void test_h264_stream_read_bytes(CuTest* tc)
{
    uint8_t data[3] = {0xae, 0xcf, 0x0b};
    h264_stream_t *s = h264_stream_new(data, 3);
    CuAssertIntEquals(tc, 0xaecf, h264_stream_read_bytes(s, 2));
    CuAssertIntEquals(tc, 0, h264_stream_read_bits(s, 2));
    CuAssertIntEquals(tc, 0x0b, h264_stream_read_bytes(s, 1));
}

CuSuite* test_h264_stream_get_suite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_h264_stream_read_bits);
    SUITE_ADD_TEST(suite, test_h264_stream_read_bytes);
    return suite;
}

