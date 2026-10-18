// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/string.h>
#include <crypto/skcipher.h>
#include <crypto/internal/skcipher.h>

#include "../xor_crypto_lib/xorcipher.h"

#define XOR_MAX_KEYSIZE 64

struct xor_skcipher_ctx {
	u8 key[XOR_MAX_KEYSIZE];
	unsigned int keylen;
};

static int xor_skcipher_setkey(struct crypto_skcipher *tfm, const u8 *key,
				unsigned int keylen)
{
	struct xor_skcipher_ctx *ctx = crypto_skcipher_ctx(tfm);

	if (keylen == 0 || keylen > XOR_MAX_KEYSIZE)
		return -EINVAL;

	memcpy(ctx->key, key, keylen);
	ctx->keylen = keylen;
	return 0;
}

static int xor_skcipher_crypt(struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	struct xor_skcipher_ctx *ctx = crypto_skcipher_ctx(tfm);
	struct skcipher_walk walk;
	int err;

	err = skcipher_walk_virt(&walk, req, false);
	if (err)
		return err;

	while (walk.nbytes) {
		xor_crypt_ecb(ctx->key, ctx->keylen,
			      walk.src.virt.addr, walk.dst.virt.addr,
			      walk.nbytes);
		err = skcipher_walk_done(&walk, 0);
		if (err)
			return err;
	}

	return 0;
}

static struct skcipher_alg xor_skcipher = {
	.setkey = xor_skcipher_setkey,
	.encrypt = xor_skcipher_crypt,
	.decrypt = xor_skcipher_crypt,

	.min_keysize = 1,
	.max_keysize = XOR_MAX_KEYSIZE,
	.ivsize = 0,

	.base = {
		.cra_name = "xorcipher",
		.cra_driver_name = "xorcipher-generic",
		.cra_priority = 0,
		.cra_blocksize = 1,
		.cra_ctxsize = sizeof(struct xor_skcipher_ctx),
		.cra_module = THIS_MODULE,
	},
};

static int __init xor_skcipher_init(void)
{
	return crypto_register_skcipher(&xor_skcipher);
}

static void __exit xor_skcipher_exit(void)
{
	crypto_unregister_skcipher(&xor_skcipher);
}

module_init(xor_skcipher_init);
module_exit(xor_skcipher_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("XOR cipher registered as a Crypto API skcipher");
