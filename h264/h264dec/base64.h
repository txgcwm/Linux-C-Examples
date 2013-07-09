/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef BASE64_H
#define BASE64_H
#include <stddef.h>

unsigned char *base64_decode(const unsigned char *src, size_t len,
                           size_t *out_len);
#endif
