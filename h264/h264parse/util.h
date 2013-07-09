#ifndef UTIL_H
#define UTIL_H

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define BITAT(x, n) ((x & (1 << n)) == (1 << n))

typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned int uint32_t;

#endif