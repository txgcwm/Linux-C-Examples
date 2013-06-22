#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h264encoder.h"

void h264_compress_init(Encoder * en, int width, int height)
{
	en->param = (x264_param_t *) malloc(sizeof(x264_param_t));
	en->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));
	x264_param_default(en->param);	//set default param
	//en->param->rc.i_rc_method = X264_RC_CQP;//设置为恒定码率
	//en->param->i_log_level = X264_LOG_NONE;
	//en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;//取空缓存区使用不死锁保证
	//en->param->i_frame_total = 0;
	//en->param->i_keyint_max = 10; 
	//en->param->i_bframe = 5; //两个参考帧之间b帧的数目
	//en->param->b_open_gop = 0;
	//en->param->i_bframe_pyramid = 0;
	//en->param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	//en->param->rc.i_bitrate = 1024 * 10;//rate 为10 kbps
	en->param->i_width = width;	//set frame width
	en->param->i_height = height;	//set frame height
	en->param->rc.i_lookahead = 0;	//表示i帧向前缓冲区
	en->param->i_fps_num = 5;	//帧率分子
	en->param->i_fps_den = 1;	//帧率分母
	en->param->i_csp = X264_CSP_I422;
	x264_param_apply_profile(en->param, x264_profile_names[0]);	//使用baseline

	if ((en->handle = x264_encoder_open(en->param)) == 0) {
		printf("x264_encoder_open error!\n");
		return;
	}
	/* Create a new pic */
	x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
					   en->param->i_height);
	en->picture->img.i_csp = X264_CSP_I422;
	en->picture->img.i_plane = 3;

	return;
}

int h264_compress_frame(Encoder * en, int type, uint8_t * in, uint8_t * out)
{
	x264_picture_t pic_out;
	int nNal = -1;
	int result = 0;
	int i = 0;
	uint8_t *p_out = out;

	char *y = en->picture->img.plane[0];
	char *u = en->picture->img.plane[1];
	char *v = en->picture->img.plane[2];

	int is_y = 1, is_u = 1;
	int y_index = 0, u_index = 0, v_index = 0;
	int yuv422_length = 2 * en->param->i_width * en->param->i_height;

	//序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
	for (i = 0; i < yuv422_length; ++i) {
		if (is_y) {
			*(y + y_index) = *(in + i);
			++y_index;
			is_y = 0;
		} else {
			if (is_u) {
				*(u + u_index) = *(in + i);
				++u_index;
				is_u = 0;
			} else {
				*(v + v_index) = *(in + i);
				++v_index;
				is_u = 1;
			}
			is_y = 1;
		}
	}

	switch (type) {
	case 0:
		en->picture->i_type = X264_TYPE_P;
		break;
	case 1:
		en->picture->i_type = X264_TYPE_IDR;
		break;
	case 2:
		en->picture->i_type = X264_TYPE_I;
		break;
	default:
		en->picture->i_type = X264_TYPE_AUTO;
		break;
	}

	//printf("en->handle->param.i_csp:%08x\n", en->handle->param.i_csp);
	if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
							&pic_out) < 0) {
		printf("x264_encoder_encode error,type:%08x!\n",
			   en->picture->img.i_csp);
		return -1;
	}

	for (i = 0; i < nNal; i++) {
		memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
		p_out += en->nal[i].i_payload;
		result += en->nal[i].i_payload;
	}

	return result;
}

void h264_compress_uninit(Encoder * en)
{
	if (en->picture) {
		x264_picture_clean(en->picture);
		free(en->picture);
		en->picture = 0;
	}
	if (en->param) {
		free(en->param);
		en->param = 0;
	}
	if (en->handle) {
		x264_encoder_close(en->handle);
	}
	free(en);

	return;
}
