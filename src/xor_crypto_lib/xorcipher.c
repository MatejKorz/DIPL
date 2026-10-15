#include <linux/module.h>
#include <linux/export.h>
#include <linux/bug.h>
#include "xorcipher.h"


void xor_crypt_ecb(const u8 *key, unsigned int keylen, const u8 *src, u8 *dst, unsigned int len) {
    unsigned int i;
    if (WARN_ON(keylen == 0)) return;

    for (i = 0; i < len; i++) {
        dst[i] = src[i] ^ key[i % keylen];
    }
}


EXPORT_SYMBOL_GPL(xor_crypt_ecb);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("XOR ECB cipher - test module");

