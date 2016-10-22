#ifndef _RTSP_SOCKET_H
#define _RTSP_SOCKET_H

#include "type.h"

S32 tcp_read(S32 fd, void *buf, S32 length);
S32 tcp_write(S32 fd, void *buf, S32 length);
void *vd_rtsp_proc(void *arg);

#endif
