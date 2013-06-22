#ifndef _TCP_SCAN_H_
#define _TCP_SCAN_H_

#include <sys/types.h>
#include <sys/socket.h>

#define TS_ACCEPTED         1
#define TS_REFUSED          2
#define TS_UNREACH          3

struct addrseg {
	int as_family;
	void *as_address;
	unsigned int as_bits;
	struct addrseg *as_next;
};

typedef int (*scan_info_t) (const struct sockaddr *, socklen_t, int, void *);

#ifdef __cplusplus
extern "C" {
#endif

	int tcp_scan(const struct addrseg *addrscope, const unsigned short *ports,
				 unsigned int ifindex, const char *ifname, int resetuid,
				 scan_info_t info, void *arg);

#ifdef __cplusplus
}
#endif
#endif
