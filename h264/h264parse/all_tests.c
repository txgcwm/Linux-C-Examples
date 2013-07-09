#include <stdio.h>

#include "CuTest.h"
#include "test_h264_stream.h"
#include "test_h264_parse.h"

void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, test_h264_stream_get_suite());
    CuSuiteAddSuite(suite, test_h264_parse_get_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    RunAllTests();
    return 0;
}