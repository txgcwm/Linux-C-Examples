#ifndef _VIDEO_CAPTURE_H_
#define _VIDEO_CAPTURE_H_

#include <linux/videodev2.h>

#include "h264encoder.h"

struct buffer {
	void *start;
	size_t length;
};

struct camera {
	char *device_name;
	int fd;
	int width;
	int height;
	int display_depth;

	char *h264_file_name;
	FILE *h264_fp;
	uint8_t *h264_buf;

	int buff_num;
	struct buffer *buffers;
	struct v4l2_capability v4l2_cap;
	struct v4l2_cropcap v4l2_cropcap;
	struct v4l2_format v4l2_fmt;
	struct v4l2_crop crop;

	Encoder en;
};

#endif
