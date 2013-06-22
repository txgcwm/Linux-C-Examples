#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#ifdef IPV6
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#endif
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <rbtree.h>
#include <unp.h> 
 
#include "tcp_scan.h"

#define TCP_SCANNER_PORT        12200
#define TCP_SCAN_REPEATS        4
#define MAX_SEG_LIFETIME        30

#ifndef offsetof
# define offsetof(type, member)     ((size_t)&((type *)0)->member)
#endif

#define SET_FD_NONBLOCK(fd) \
( {\
int __flags = fcntl(fd, F_GETFL, 0);\
if (__flags >= 0)\
	__flags = fcntl(fd, F_SETFL, __flags | O_NONBLOCK);\
__flags;\
})

#ifdef IPV6
# define IN6_ADDR_NEXT(addr) \
	do {\
int __i;\
for (__i = 15; __i >= 0; __i--)\
	\
{\
if (++((unsigned char *)(addr))[__i] != 0)\
	break;\
}\
}\
while (0)
#endif

#ifndef __linux__
# define select(maxfdp1, rset, wset, xset, timeout) \
	( {\
	struct timeval __tv = *(timeout);\
long __tps = sysconf(_SC_CLK_TCK);\
clock_t __clock = times(NULL);\
int __n = select(maxfdp1, rset, wset, xset, timeout);\
__clock = times(NULL) - __clock;\
*(timeout) = __tv;\
if (((timeout)->tv_sec -= __clock / __tps) < 0)\
	{\
	(timeout)->tv_usec += 1000000 * (timeout)->tv_sec;\
	(timeout)->tv_sec = 0;\
	}\
if (((timeout)->tv_usec -= __clock % __tps * 1000000 / __tps) < 0)\
	(timeout)->tv_usec = 0;\
__n;\
})
#endif

/* "sockaddr" structs arranged in a Red-Black tree. */
struct __sockaddr_rb {
	struct rb_node rb;
	socklen_t addrlen;
	struct sockaddr sockaddr;
};

/* Union of sockaddr. */
union __sa_union {
	struct sockaddr sockaddr;
	struct sockaddr_in sin;
#ifdef IPV6
	struct sockaddr_in6 sin6;
#endif
};

/* IP pseudoheader of TCP or UDP. */
struct __ip_pheader {
	struct in_addr src;
	struct in_addr dst;
	unsigned char zero;
	unsigned char protocol;
	unsigned short len;
};

#ifdef IPV6
struct __ip6_pheader {
	struct in6_addr src;
	struct in6_addr dst;
	unsigned int len;
	unsigned short zero1;
	unsigned char zero2;
	unsigned char next;
};
#endif

/* TCP header combined with IP pseudoheader. */
struct __tcp_pheader {
#ifdef IPV6
	union {
		struct __ip6_pheader phdr6;
		struct {
			char __pad[sizeof(struct __ip6_pheader) -
					   sizeof(struct __ip_pheader)];
			struct __ip_pheader phdr;
		};
	};
#else
	struct __ip_pheader phdr;
#endif
	struct tcphdr tcphdr;
};

unsigned short __tcp_scanner_port = TCP_SCANNER_PORT;
int __tcp_scan_repeats = TCP_SCAN_REPEATS;
int __max_seg_lifetime = MAX_SEG_LIFETIME;

/* For the sake of simplicity. */
static unsigned int __tcp_header_seq;
static scan_info_t __info;
static void *__arg;

inline static void __make_tcp_ip_pheader(const struct in_addr *saddr,
										 const struct in_addr *daddr,
										 struct __ip_pheader *phdr)
{
	phdr->src = *saddr;
	phdr->dst = *daddr;
	phdr->zero = 0;
	phdr->protocol = IPPROTO_TCP;
	phdr->len = htons(sizeof(struct tcphdr));
}

#ifdef IPV6
inline static void __make_tcp_ip6_pheader(const struct in6_addr *saddr,
										  const struct in6_addr *daddr,
										  struct __ip6_pheader *phdr)
{
	phdr->src = *saddr;
	phdr->dst = *daddr;
	phdr->len = htonl(sizeof(struct tcphdr));
	phdr->zero1 = 0;
	phdr->zero2 = 0;
	phdr->next = IPPROTO_TCP;
}
#endif

/* Send a TCP packet. (Without TCP options or TCP data.) */
static int __tcp_send(int sockfd, struct __tcp_pheader *tcpphdr,
					  const void *saddr, const struct sockaddr *daddr,
					  socklen_t addrlen)
{
	unsigned short *pfrom;
	int len;

	switch (daddr->sa_family) {
	case AF_INET:
		__make_tcp_ip_pheader((const struct in_addr *)saddr,
							  &((const struct sockaddr_in *)
								daddr)->sin_addr, &tcpphdr->phdr);
		pfrom = (unsigned short *)&tcpphdr->phdr;
		len = sizeof(struct __ip_pheader) + sizeof(struct tcphdr);
		break;
#ifdef IPV6
	case AF_INET6:
		__make_tcp_ip6_pheader((const struct in6_addr *)saddr,
							   &((const struct sockaddr_in6 *)
								 daddr)->sin6_addr, &tcpphdr->phdr6);
		pfrom = (unsigned short *)&tcpphdr->phdr6;
		len = sizeof(struct __ip6_pheader) + sizeof(struct tcphdr);
		break;
#endif
	default:
		errno = EAFNOSUPPORT;
		return -1;
	}

	tcpphdr->tcphdr.th_sum = 0;
	tcpphdr->tcphdr.th_sum = in_cksum(pfrom, len);
	return sendto(sockfd, &tcpphdr->tcphdr, sizeof(struct tcphdr), 0,
				  daddr, addrlen);
}

/* Send a SYN packet to the remote server. */
static int __tcp_syn(int sockfd, unsigned short sport, unsigned short dport,
					 const void *saddr, const struct sockaddr *daddr,
					 socklen_t addrlen)
{
	struct __tcp_pheader tcpphdr = {
	  tcphdr:{
		  th_sport: htons(sport), th_dport: htons(dport), th_seq: htonl(__tcp_header_seq), th_ack: htonl(0), th_x2: 0, th_off: sizeof(struct tcphdr) >> 2, th_flags: TH_SYN, th_win: htons(8192), th_urp:htons
		 (0)
		 }
	};

	return __tcp_send(sockfd, &tcpphdr, saddr, daddr, addrlen);
}

/* Tell the remote server that connection is aborted. */
static int __tcp_rst(int sockfd, unsigned short sport, unsigned short dport,
					 const void *saddr, const struct sockaddr *daddr,
					 socklen_t addrlen)
{
	struct __tcp_pheader tcpphdr = {
	  tcphdr:{
		  th_sport: htons(sport), th_dport: htons(dport), th_seq: htonl(0), th_ack: htonl(0), th_x2: 0, th_off: sizeof(struct tcphdr) >> 2, th_flags: TH_RST, th_win: htons(0), th_urp:htons
		 (0)
		 }
	};

	return __tcp_send(sockfd, &tcpphdr, saddr, daddr, addrlen);
}

/* Search a sockaddr in a Red-Black tree. */
static struct __sockaddr_rb *__rb_search_sockaddr(const struct sockaddr
												  *sockaddr, socklen_t addrlen, const struct rb_root
												  *root)
{
	const struct rb_node *p = root->node;
	struct __sockaddr_rb *entry;
	int n;

	while (p) {
		entry = rb_entry(p, struct __sockaddr_rb, rb);
		if (addrlen < entry->addrlen)
			n = -1;
		else if (addrlen > entry->addrlen)
			n = 1;
		else if ((n = memcmp(sockaddr, &entry->sockaddr, addrlen)) == 0)
			return entry;

		p = n < 0 ? p->left : p->right;
	}

	return NULL;
}

/* Insert a sockaddr into a Red-Black tree. */
static struct __sockaddr_rb *__rb_insert_sockaddr(const struct sockaddr
												  *sockaddr,
												  socklen_t addrlen,
												  struct rb_root *root)
{
	struct rb_node **p = &root->node;
	struct rb_node *parent = NULL;
	struct __sockaddr_rb *entry;
	int n;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct __sockaddr_rb, rb);
		if (addrlen < entry->addrlen)
			n = -1;
		else if (addrlen > entry->addrlen)
			n = 1;
		else if ((n = memcmp(sockaddr, &entry->sockaddr, addrlen)) == 0)
			return entry;

		p = n < 0 ? &parent->left : &parent->right;
	}

#define __SOCKADDR_RB_SIZE \
		(offsetof(struct __sockaddr_rb, sockaddr)+addrlen)
	if (entry = (struct __sockaddr_rb *)malloc(__SOCKADDR_RB_SIZE)) {
		entry->addrlen = addrlen;
		memcpy(&entry->sockaddr, sockaddr, addrlen);
		rb_link_node(&entry->rb, parent, p);
		rb_insert_color(&entry->rb, root);
		return NULL;
	}
#undef __SOCKADDR_RB_SIZE

	return (struct __sockaddr_rb *)-1;
}

static void __rb_destroy_sockaddr(struct rb_root *root)
{
	struct __sockaddr_rb *entry;

	while (root->node) {
		entry = rb_entry(root->node, struct __sockaddr_rb, rb);
		rb_erase(root->node, root);
		free(entry);
	}
}

static int __proc_tcp_packet(int sockfd, const struct tcphdr *tcphdr,
							 const void *daddr,
							 const struct sockaddr *saddr,
							 socklen_t addrlen, struct rb_root *root)
{
	struct __sockaddr_rb *entry;
	int state;

	if (ntohs(tcphdr->th_dport) == __tcp_scanner_port) {
		if (tcphdr->th_flags & (TH_SYN | TH_RST) &&
			tcphdr->th_flags & TH_ACK &&
			ntohl(tcphdr->th_ack) == __tcp_header_seq + 1) {
			if (tcphdr->th_flags & TH_SYN) {
				/* Be a polite scanner! */
				__tcp_rst(sockfd, __tcp_scanner_port,
						  ntohs(tcphdr->th_sport), daddr, saddr, addrlen);
			}

			if (!(entry = __rb_insert_sockaddr(saddr, addrlen, root))) {
				state = tcphdr->th_flags & TH_SYN ? TS_ACCEPTED : TS_REFUSED;
				if (__info(saddr, addrlen, state, __arg) < 0)
					return -1;
			} else if (entry == (struct __sockaddr_rb *)-1)
				return -1;
		}
	}

	return 0;
}

static int __recv_tcp_packet(int sockfd, struct rb_root *root)
{
	/* We do not care the TCP options or TCP data, so the buffer size is the  
	   biggest IPv4 header size, which is bigger than IPv6 header size, plus  
	   TCP header size. */
#define __BUFSIZE   ((0xf << 2) + sizeof (struct tcphdr))
	char buf[__BUFSIZE];
	struct ip *iphdr = (struct ip *)buf;
#ifdef IPV6
	struct ip6_hdr *ip6hdr = (struct ip6_hdr *)buf;
#endif
	struct tcphdr *tcphdr;
	union __sa_union un;
	socklen_t addrlen;
	ssize_t n;

	while (addrlen = sizeof(union __sa_union), (n = recvfrom(sockfd, buf,
															 __BUFSIZE, 0,
															 &un.sockaddr,
															 &addrlen)) >= 0) {
		if (un.sockaddr.sa_family == AF_INET) {
			/* Make sure that it is a valid TCP packet. */
			if (n >= sizeof(struct ip) && iphdr->ip_p == IPPROTO_TCP &&
				n >= (iphdr->ip_hl << 2) + sizeof(struct tcphdr)) {
				tcphdr = (struct tcphdr *)(buf + (iphdr->ip_hl << 2));
				un.sin.sin_port = tcphdr->th_sport;
				if (__proc_tcp_packet(sockfd, tcphdr, &iphdr->ip_dst,
									  &un.sockaddr, addrlen, root) < 0)
					break;
			}
		}
#ifdef IPV6
		else if (un.sockaddr.sa_family == AF_INET6) {
			if (n >= sizeof(struct ip6_hdr) + sizeof(struct tcphdr) &&
				ip6hdr->ip6_nxt == IPPROTO_TCP) {
				tcphdr = (struct tcphdr *)(ip6hdr + 1);
				un.sin6.sin6_port = tcphdr->th_sport;
				if (__proc_tcp_packet(sockfd, tcphdr, &ip6hdr->ip6_dst,
									  &un.sockaddr, addrlen, root) < 0)
					break;
			}
		}
#endif
	}

#undef __BUFSIZE
	return n < 0 && errno == EAGAIN ? 0 : -1;
}

static int __proc_icmp_packet(const struct icmp *icmp, size_t icmplen,
							  struct sockaddr *saddr, socklen_t addrlen,
							  struct rb_root *root)
{
	struct __sockaddr_rb *entry;
	const struct ip *iphdr;
	const struct tcphdr *tcphdr;

	/* We care only ICMP unreach packet. */
	if (icmp->icmp_type == ICMP_UNREACH) {
		/*  if (icmplen >= sizeof (struct icmp) + sizeof (struct ip))   */
		if (icmplen >= offsetof(struct icmp, icmp_data) + sizeof(struct ip)) {
			iphdr = (const struct ip *)icmp->icmp_data;
			if (icmplen >= offsetof(struct icmp, icmp_data) +
				(iphdr->ip_hl << 2) + 8) {
				tcphdr = (const struct tcphdr *)((const char *)iphdr +
												 (iphdr->ip_hl << 2));
				if (ntohs(tcphdr->th_sport) == __tcp_scanner_port &&
					ntohl(tcphdr->th_seq) == __tcp_header_seq) {
					((struct sockaddr_in *)saddr)->sin_port = tcphdr->th_dport;
					if (!(entry = __rb_insert_sockaddr(saddr, addrlen, root))) {
						if (__info(saddr, addrlen, TS_UNREACH, __arg) < 0)
							return -1;
					} else if (entry == (struct __sockaddr_rb *)-1)
						return -1;
				}
			}
		}
	}

	return 0;
}

#ifdef IPV6
static int __proc_icmp6_packet(const struct icmp6_hdr *icmp6,
							   size_t icmplen, struct sockaddr *saddr,
							   socklen_t addrlen, struct rb_root *root)
{
	struct __sockaddr_rb *entry;
	const struct ip6_hdr *ip6hdr;
	const struct tcphdr *tcphdr;

	if (icmp6->icmp6_type == ICMP6_DST_UNREACH) {
		if (icmplen >= sizeof(struct icmp6_hdr) + sizeof(struct ip6_hdr) + 8) {
			ip6hdr = (const struct ip6_hdr *)(icmp6 + 1);
			tcphdr = (const struct tcphdr *)(ip6hdr + 1);
			if (ntohs(tcphdr->th_sport) == __tcp_scanner_port &&
				ntohl(tcphdr->th_seq) == __tcp_header_seq) {
				((struct sockaddr_in6 *)saddr)->sin6_port = tcphdr->th_dport;
				if (!(entry = __rb_insert_sockaddr(saddr, addrlen, root))) {
					if (__info(saddr, addrlen, TS_UNREACH, __arg) < 0)
						return -1;
				} else if (entry == (struct __sockaddr_rb *)-1)
					return -1;
			}
		}
	}

	return 0;
}
#endif

static int __recv_icmp_packet(int sockfd, struct rb_root *root)
{
	/* Biggest IPv4 header, ICMP header, the returned IP header with  
	   8 bytes TCP header. */
#define __BUFSIZE \
		((0xf << 2) + offsetof(struct icmp, icmp_data) + (0xf << 2) + 8)
	char buf[__BUFSIZE];
	struct ip *iphdr = (struct ip *)buf;
#ifdef IPV6
	struct ip6_hdr *ip6hdr = (struct ip6_hdr *)buf;
#endif
	union __sa_union un;
	socklen_t addrlen;
	ssize_t n;

	while (addrlen = sizeof(union __sa_union), (n = recvfrom(sockfd, buf,
															 __BUFSIZE, 0,
															 &un.sockaddr,
															 &addrlen)) >= 0) {
		if (un.sockaddr.sa_family == AF_INET) {
			if (n >= sizeof(struct ip) && iphdr->ip_p == IPPROTO_ICMP &&
				n >= (iphdr->ip_hl << 2) + offsetof(struct icmp, icmp_data)) {
				struct icmp *icmp = (struct icmp *)(buf + (iphdr->ip_hl << 2));
				if (__proc_icmp_packet
					(icmp, n - (iphdr->ip_hl << 2), &un.sockaddr, addrlen,
					 root) < 0)
					break;
			}
		}
#ifdef IPV6
		else if (un.sockaddr.sa_family == AF_INET6) {
			if (n >= sizeof(struct ip6_hdr) + sizeof(struct icmp6_hdr) &&
				ip6hdr->ip6_nxt == IPPROTO_ICMPV6) {
				struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)(ip6hdr + 1);
				if (__proc_icmp6_packet(icmp6, n - sizeof(struct ip6_hdr),
										&un.sockaddr, addrlen, root) < 0)
					break;
			}
		}
#endif
	}

#undef __BUFSIZE
	return n < 0 && errno == EAGAIN ? 0 : -1;
}

inline static int
#ifdef IPV6
__scan_ip_seg(const struct in_addr *address, unsigned int bits,
			  unsigned short port, int tcpsock, int icmpsock,
			  int tcpsock6, int icmpsock6, const struct in_addr *myaddr,
			  struct rb_root *root)
#else
__scan_ip_seg(const struct in_addr *address, unsigned int bits,
			  unsigned short port, int tcpsock, int icmpsock,
			  const struct in_addr *myaddr, struct rb_root *root)
#endif
{
	unsigned int hostmax = 0;
	unsigned int host;
	struct sockaddr_in sin;
	unsigned int bit = 1;

	bzero(&sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	for (host = 0; host < 32 - bits; host++) {
		hostmax |= bit;
		bit <<= 1;
	}

	host = ntohl(address->s_addr) & ~hostmax;
	hostmax |= ntohl(address->s_addr);
	do {
		sin.sin_addr.s_addr = htonl(host);
		if (!__rb_search_sockaddr((struct sockaddr *)&sin,
								  sizeof(struct sockaddr_in), root)) {
			__tcp_syn(tcpsock, __tcp_scanner_port, port,
					  myaddr, (struct sockaddr *)&sin,
					  sizeof(struct sockaddr_in));
		}

		if (__recv_tcp_packet(tcpsock, root) < 0)
			return -1;
		if (__recv_icmp_packet(icmpsock, root) < 0)
			return -1;
#ifdef IPV6
		if (tcpsock6 >= 0) {
			if (__recv_tcp_packet(tcpsock6, root) < 0)
				return -1;
			if (__recv_icmp_packet(icmpsock6, root) < 0)
				return -1;
		}
#endif
	} while (host++ != hostmax);

	return 0;
}

#ifdef IPV6
inline static int
__scan_ip6_seg(const struct in6_addr *address, unsigned int bits,
			   unsigned short port, int tcpsock, int icmpsock,
			   int tcpsock6, int icmpsock6, const struct in6_addr *myaddr,
			   struct rb_root *root)
{
	struct in6_addr hostmax;
	struct in6_addr host;
	struct sockaddr_in6 sin6;
	unsigned char bit = 1;
	int i, j;

	bzero(&hostmax, sizeof(struct in6_addr));
	for (i = 15; i > bits >> 3; i--)
		hostmax.s6_addr[i] = 0xff;

	for (j = 0; j < (128 - bits) & 7; j++) {
		hostmax.s6_addr[i] |= bit;
		bit <<= 1;
	}

	for (i = 0; i < 4; i++) {
		host.s6_addr32[i] = address->s6_addr32[i] & ~hostmax.s6_addr32[i];
		hostmax.s6_addr32[i] |= address->s6_addr32[i];
	}

	bzero(&sin6, sizeof(struct sockaddr_in6));
	sin6.sin6_family = AF_INET6;
	while (1) {
		sin6.sin6_addr = host;
		if (!__rb_search_sockaddr((struct sockaddr *)&sin6,
								  sizeof(struct sockaddr_in6), root)) {
			__tcp_syn(tcpsock, __tcp_scanner_port, port,
					  myaddr, (struct sockaddr *)&sin6,
					  sizeof(struct sockaddr_in6));
		}

		if (tcpsock >= 0) {
			if (__recv_tcp_packet(tcpsock, root) < 0)
				return -1;
			if (__recv_icmp_packet(icmpsock, root) < 0)
				return -1;
		}

		if (__recv_tcp_packet(tcpsock6, root) < 0)
			return -1;
		if (__recv_icmp_packet(icmpsock6, root) < 0)
			return -1;

		if (IN6_ARE_ADDR_EQUAL(&host, &hostmax))
			break;
		IN6_ADDR_NEXT(&host);
	}

	return 0;
}
#endif

#ifdef IPV6
static int __wait_response(int tcpsock, int icmpsock, int tcpsock6,
						   int icmpsock6, struct rb_root *root)
#else
static int __wait_response(int tcpsock, int icmpsock, struct rb_root *root)
#endif
{
	fd_set all, rset;
	struct timeval timeout = {
	  tv_sec: __max_seg_lifetime << 1, tv_usec:0
	};
	int maxfd = -1;
	int n;

#define __max(x, y)     ((x) > (y) ? (x) : (y))
	if (tcpsock >= 0) {
		FD_SET(tcpsock, &all);
		FD_SET(icmpsock, &all);
		maxfd = __max(tcpsock, icmpsock);
	}
#ifdef IPV6
	if (tcpsock6 >= 0) {
		FD_SET(tcpsock6, &all);
		FD_SET(icmpsock6, &all);
		maxfd = __max(maxfd, __max(tcpsock6, icmpsock6));
	}
#endif
#undef __max

	/* Wait at least 2*MSL for all IP segments to expire. */
	while (rset = all, (n = select(maxfd + 1, &rset, NULL, NULL, &timeout)) > 0) {
		if (tcpsock >= 0) {
			if (FD_ISSET(tcpsock, &rset)) {
				if (__recv_tcp_packet(tcpsock, root) < 0)
					return -1;
			}
			if (FD_ISSET(icmpsock, &rset)) {
				if (__recv_icmp_packet(icmpsock, root) < 0)
					return -1;
			}
		}
#ifdef IPV6
		if (tcpsock6 >= 0) {
			if (FD_ISSET(tcpsock6, &rset)) {
				if (__recv_tcp_packet(tcpsock6, root) < 0)
					return -1;
			}
			if (FD_ISSET(icmpsock6, &rset)) {
				if (__recv_icmp_packet(icmpsock6, root) < 0)
					return -1;
			}
		}
#endif
	}

	return n;
}

int tcp_scan(const struct addrseg *scope, const unsigned short *ports,
			 unsigned int ifindex, const char *ifname, int resetuid,
			 scan_info_t info, void *arg)
{
	const struct addrseg *p;
	const short *port;
	int tcpsock = -1, icmpsock = -1;
	struct in_addr ipaddr;
#ifdef IPV6
	int tcpsock6 = -1, icmpsock6 = -1;
	struct in6_addr ip6addr;
#endif
	union __sa_union un;
	int i, ret = -1;
	struct rb_root root = RB_ROOT;

	for (p = scope; p; p = p->as_next) {
		switch (p->as_family) {
		case AF_INET:
			tcpsock = -2;
			break;
#ifdef IPV6
		case AF_INET6:
			tcpsock6 = -2;
			break;
#endif
		default:
			errno = EAFNOSUPPORT;
			return -1;
		}
	}

	if (tcpsock == -2) {
		if (getifaddr(AF_INET, ifindex, ifname, &ipaddr) < 0)
			goto error;
		if ((tcpsock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)
			goto error;
		if ((icmpsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
			goto error;
		bzero(&un.sin, sizeof(struct sockaddr_in));
		un.sin.sin_family = AF_INET;
		un.sin.sin_addr = ipaddr;
		if (bind(tcpsock, &un.sockaddr, sizeof(struct sockaddr_in)) < 0)
			goto error;
		if (bind(icmpsock, &un.sockaddr, sizeof(struct sockaddr_in)) < 0)
			goto error;
		if (SET_FD_NONBLOCK(tcpsock) < 0)
			goto error;
		if (SET_FD_NONBLOCK(icmpsock) < 0)
			goto error;
	}
#ifdef IPV6
	if (tcpsock6 == -2) {
		if (getifaddr(AF_INET6, ifindex, ifname, &ip6addr) < 0)
			goto error;
		if ((tcpsock6 = socket(AF_INET6, SOCK_RAW, IPPROTO_TCP)) < 0)
			goto error;
		if ((icmpsock6 = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
			goto error;
		bzero(&un.sin6, sizeof(struct sockaddr_in6));
		un.sin6.sin6_family = AF_INET6;
		un.sin6.sin6_addr = ip6addr;
		if (bind(tcpsock6, &un.sockaddr, sizeof(struct sockaddr_in6)) < 0)
			goto error;
		if (bind(icmpsock6, &un.sockaddr, sizeof(struct sockaddr_in6)) < 0)
			goto error;
		if (SET_FD_NONBLOCK(tcpsock6) < 0)
			goto error;
		if (SET_FD_NONBLOCK(icmpsock6) < 0)
			goto error;
	}
#endif

	/* We no longer need root privilege. */
	if (resetuid)
		setuid(getuid());

	/* Generate a random TCP sequance. */
	srand(time(NULL));
	__tcp_header_seq = rand();

	__info = info;
	__arg = arg;
	for (i = 0; i < __tcp_scan_repeats; i++) {
		for (port = ports; *port; port++) {
			for (p = scope; p; p = p->as_next) {
				if (p->as_family == AF_INET) {
#ifdef IPV6
					if (__scan_ip_seg(p->as_address, p->as_bits, *port,
									  tcpsock, icmpsock, tcpsock6,
									  icmpsock6, &ipaddr, &root) < 0)
						goto error;
#else
					if (__scan_ip_seg(p->as_address, p->as_bits,
									  *port, tcpsock, icmpsock,
									  &ipaddr, &root) < 0)
						goto error;
#endif
				}
#ifdef IPV6
				else {			/* if (p->as_family == AF_INET6) */

					if (__scan_ip6_seg(p->as_address, p->as_bits, *port,
									   tcpsock, icmpsock, tcpsock6,
									   icmpsock6, &ip6addr, &root) < 0)
						goto error;
				}
#endif
			}
		}
	}

#ifdef IPV6
	if (__wait_response(tcpsock, icmpsock, tcpsock6, icmpsock6, &root) >= 0)
		ret = 0;
#else
	if (__wait_response(tcpsock, icmpsock, &root) >= 0)
		ret = 0;
#endif

  error:
	__rb_destroy_sockaddr(&root);
	if (tcpsock >= 0)
		close(tcpsock);
	if (icmpsock >= 0)
		close(icmpsock);
#ifdef IPV6
	if (tcpsock6 >= 0)
		close(tcpsock6);
	if (icmpsock6 >= 0)
		close(icmpsock6);
#endif

	return ret;
}
