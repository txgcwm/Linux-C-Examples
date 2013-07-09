OBJS = h264_stream.o h264_nal_unit.o h264_parse.o h264_sequence_parameter_set.o h264_pic_parameter_set.o h264_vui_parameters.o h264_slice.o
TEST_OBJS = CuTest.o test_h264_stream.o test_h264_parse.o
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

h264 : $(OBJS) main.o all_tests
	$(CC) $(LFLAGS) $(OBJS) main.o -o h264

main.o : main.c
	$(CC) $(CFLAGS) main.c

h264_stream.o : h264_stream.h h264_stream.c
	$(CC) $(CFLAGS) h264_stream.c

h264_vui_parameters.o: h264_vui_parameters.h h264_vui_parameters.c
	$(CC) $(CFLAGS) h264_vui_parameters.c

h264_sequence_parameter_set.o : h264_sequence_parameter_set.h h264_sequence_parameter_set.c
	$(CC) $(CFLAGS) h264_sequence_parameter_set.c

h264_pic_parameter_set.o : h264_pic_parameter_set.h h264_pic_parameter_set.c
		$(CC) $(CFLAGS) h264_pic_parameter_set.c

h264_slice.o : h264_slice.h h264_slice.c
		$(CC) $(CFLAGS) h264_slice.c

CuTest.o: CuTest.h CuTest.c
	$(CC) $(CFLAGS) CuTest.c

test_h264_stream.o: test_h264_stream.h test_h264_stream.c
	$(CC) $(CFLAGS) test_h264_stream.c

test_h264_parse.o: test_h264_parse.h test_h264_parse.c
	$(CC) $(CFLAGS) test_h264_parse.c

all_tests.o: all_tests.c
	$(CC) $(CFLAGS) all_tests.c

all_tests: h264_stream.o CuTest.o test_h264_stream.o h264_parse.o test_h264_parse.o all_tests.o
	$(CC) $(LFLAGS) $(OBJS) $(TEST_OBJS) all_tests.o -o all_tests

h264_nal_unit.o : h264_nal_unit.h h264_nal_unit.c
	$(CC) $(CFLAGS) h264_nal_unit.c

h264_parse.o : h264_parse.h h264_parse.c
	$(CC) $(CFLAGS) h264_parse.c

clean:
	\rm *.o h264
