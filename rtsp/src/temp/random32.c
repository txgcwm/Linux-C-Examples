/* * 
 * This file is part of libnemesi
 *
 * Copyright (C) 2007 by LScube team <team@streaming.polito.it>
 * See AUTHORS for more details
 * 
 * libnemesi is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libnemesi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libnemesi; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *  
 * */

/* From RFC 1889 */

#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include "type.h"

#ifndef WIN32
#       include <sys/utsname.h>
#       include <sys/types.h>
#       include <unistd.h>
#endif

#ifdef HAVE_BYTESWAP_H
#       include <byteswap.h>
#else
#	ifndef bswap_16
#		define bswap_16(value) ((((value) & 0xff) << 8) | ((value) >> 8))
#	endif
#	ifndef bswap_32
#		define bswap_32(value) (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16)\
                                        | (uint32_t)bswap_16((uint16_t)((value) >> 16)))
#	endif
#	ifndef bswap_64
#		define bswap_64(value) (((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) << 32)\
                                        | (uint64_t)bswap_32((uint32_t)((value) >> 32)))
#	endif
#endif
//shamelessly ripping avutils md5

#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

typedef struct AVMD5{
    uint64_t len;
    uint8_t  block[64];
    uint32_t ABCD[4];
} AVMD5;

static const uint8_t S[4][4] = {
    { 7, 12, 17, 22 },  /* Round 1 */
    { 5,  9, 14, 20 },  /* Round 2 */
    { 4, 11, 16, 23 },  /* Round 3 */
    { 6, 10, 15, 21 }   /* Round 4 */
};

static const uint32_t T[64] = { // T[i]= fabs(sin(i+1)<<32)
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,   /* Round 1 */
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,   /* Round 2 */
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,   /* Round 3 */
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,   /* Round 4 */
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

#define CORE(i, a, b, c, d) \
        t = S[i>>4][i&3];\
        a += T[i];\
\
        if(i<32){\
            if(i<16) a += (d ^ (b&(c^d))) + X[      i &15 ];\
            else     a += (c ^ (d&(c^b))) + X[ (1+5*i)&15 ];\
        }else{\
            if(i<48) a += (b^c^d)         + X[ (5+3*i)&15 ];\
            else     a += (c^(b|~d))      + X[ (  7*i)&15 ];\
        }\
        a = b + (( a << t ) | ( a >> (32 - t) ));

static VOID body(uint32_t ABCD[4], uint32_t X[16]){

    S32 t;
#ifdef WORDS_BIGENDIAN
    S32 i;
#endif    
    U32 a= ABCD[3];
    U32 b= ABCD[2];
    U32 c= ABCD[1];
    U32 d= ABCD[0];

#ifdef WORDS_BIGENDIAN
    for(i=0; i<16; i++)
        X[i]= bswap_32(X[i]);
#endif

#define CORE2(i) CORE(i,a,b,c,d) CORE((i+1),d,a,b,c) CORE((i+2),c,d,a,b) CORE((i+3),b,c,d,a)
#define CORE4(i) CORE2(i) CORE2((i+4)) CORE2((i+8)) CORE2((i+12))
CORE4(0) CORE4(16) CORE4(32) CORE4(48)

    ABCD[0] += d;
    ABCD[1] += c;
    ABCD[2] += b;
    ABCD[3] += a;
}

static VOID av_md5_init(AVMD5 *ctx){
    ctx->len    = 0;

    ctx->ABCD[0] = 0x10325476;
    ctx->ABCD[1] = 0x98badcfe;
    ctx->ABCD[2] = 0xefcdab89;
    ctx->ABCD[3] = 0x67452301;
}

static VOID av_md5_update(AVMD5 *ctx, const uint8_t *src, const S32 len){
    S32 i, j;

    j= ctx->len & 63;
    ctx->len += len;

    for( i = 0; i < len; i++ ){
        ctx->block[j++] = src[i];
        if( 64 == j ){
            body(ctx->ABCD, (uint32_t*) ctx->block);
            j = 0;
        }
    }
}

static VOID av_md5_final(AVMD5 *ctx, uint8_t *dst){
    S32 i;
    uint64_t finalcount= le2me_64(ctx->len<<3);

    av_md5_update(ctx, (uint8_t *)"\200", 1);
    while((ctx->len & 63)<56)
        av_md5_update(ctx, (uint8_t *)"", 1);

    av_md5_update(ctx, (uint8_t *)&finalcount, 8);

    for(i=0; i<4; i++)
        ((uint32_t*)dst)[i]= le2me_32(ctx->ABCD[3-i]);
}

static VOID av_md5_sum(uint8_t *dst, const uint8_t *src, const S32 len){
    AVMD5 ctx[1];

    av_md5_init(ctx);
    av_md5_update(ctx, src, len);
    av_md5_final(ctx, dst);
}

static uint32_t md_32(CHAR *string, S32 length)
{
    S32 hash[4];
    av_md5_sum((uint8_t *)hash, (uint8_t *) string, length);
    return hash[0]^hash[1]^hash[2]^hash[3];
}

#ifndef WIN32
uint32_t random32(S32 type)
{
    struct {
        S32 type;
        struct timeval tv;
        clock_t cpu;
        pid_t pid;
        uint32_t hid;
        uid_t uid;
        gid_t gid;
        struct utsname name;
    } s;

    gettimeofday(&s.tv, NULL);
    uname(&s.name);
    s.type = type;
    s.cpu = clock();
    s.pid = getpid();
    s.hid = gethostid();
    s.uid = getuid();
    s.gid = getgid();

    return md_32((CHAR *) &s, sizeof(s));
}
#else
U32 random32(S32 type)
{
    CHAR s[256];
    return md_32(s, sizeof(s));
}
#endif
