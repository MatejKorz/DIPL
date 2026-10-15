#ifndef _XORCIPHER_H
#define _XORCIPHER_H

#include <linux/types.h>

void xor_crypt_ecb(const u8 *key, unsigned int keylen, const u8 *src, u8 *dst, unsigned int len);

#endif
