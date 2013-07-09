#include <stdlib.h>

#include "CuTest.h"
#include "h264_parse.h"
#include "test_h264_parse.h"

void test_h264_ue(CuTest* tc)
{
    uint8_t data[4] = {0xa6, 0x42, 0x98, 0xe0};
    h264_stream_t *s = h264_stream_new(data, 4);
    CuAssertIntEquals(tc, 0, h264_ue(s));
    CuAssertIntEquals(tc, 1, h264_ue(s));
    CuAssertIntEquals(tc, 2, h264_ue(s));
    CuAssertIntEquals(tc, 3, h264_ue(s));
    CuAssertIntEquals(tc, 4, h264_ue(s));
    CuAssertIntEquals(tc, 5, h264_ue(s));
    CuAssertIntEquals(tc, 6, h264_ue(s));
}

CuSuite* test_h264_parse_get_suite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_h264_ue);
    return suite;
}
