#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>

#include "capture.h"

int main(int argc, char **argv)
{
	struct camera *cam = NULL;

	cam = (struct camera *)malloc(sizeof(struct camera));
	if (!cam) {
		printf("malloc camera failure!\n");
		exit(1);
	}

	memset(cam, 0, sizeof(struct camera));
	cam->device_name = "/dev/video0";
	cam->buffers = NULL;
	cam->width = 640;
	cam->height = 480;
	cam->display_depth = 5;		/* RGB24 */
	cam->h264_file_name = "test.h264";

	camera_open(cam);
	camera_init(cam);
	camera_capturing_start(cam);
	h264_compress_init(&cam->en, cam->width, cam->height);
	cam->h264_buf = (uint8_t *) malloc(sizeof(uint8_t) * cam->width * cam->height * 3);	// 设置缓冲区
	if ((cam->h264_fp = fopen(cam->h264_file_name, "wa+")) == NULL) {
		printf("open file error!\n");
		return -1;
	}

	while (1) {
		if (read_and_encode_frame(cam) < 0) {
			fprintf(stderr, "read_fram fail in thread\n");
			//break;
		}
	}

	printf("-----------end program------------");
	if (cam->h264_fp != NULL)
		fclose(cam->h264_fp);
	h264_compress_uninit(&cam->en);
	free(cam->h264_buf);

	camera_capturing_stop(cam);
	camera_uninit(cam);
	camera_close(cam);

	free(cam);

	return 0;
}
