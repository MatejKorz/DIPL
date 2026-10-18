// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <crypto/skcipher.h>

static int __init xorapi_test_init(void)
{
    static const u8 key[] = "mysecretkey";
    static const u8 plaintext[] = "Hello, lib/crypto world!";

    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct scatterlist *sg;
    u8 *src, *ciphertext, *decrypted;
    int err;

    tfm = crypto_alloc_skcipher("xorcipher", 0, 0);
    if (IS_ERR(tfm)) {
        pr_err("xorapi_test: cannot allocate xorcipher tfm: %ld\n",
               PTR_ERR(tfm));
        return PTR_ERR(tfm);
    }

    err = crypto_skcipher_setkey(tfm, key, sizeof(key) - 1);
    if (err) {
        pr_err("xorapi_test: setkey failed: %d\n", err);
        crypto_free_skcipher(tfm);
        return err;
    }

    req = kmalloc(sizeof(*req) + crypto_skcipher_reqsize(tfm), GFP_KERNEL);
    sg = kmalloc_array(2, sizeof(*sg), GFP_KERNEL);
    src = kmemdup(plaintext, sizeof(plaintext), GFP_KERNEL);
    ciphertext = kmalloc(sizeof(plaintext), GFP_KERNEL);
    decrypted = kmalloc(sizeof(plaintext), GFP_KERNEL);
    if (!req || !sg || !src || !ciphertext || !decrypted) {
        err = -ENOMEM;
        goto out;
    }

    skcipher_request_set_tfm(req, tfm);

    sg_init_one(&sg[0], src, sizeof(plaintext));
    sg_init_one(&sg[1], ciphertext, sizeof(plaintext));
    skcipher_request_set_crypt(req, &sg[0], &sg[1], sizeof(plaintext), NULL);

    err = crypto_skcipher_encrypt(req);
    if (err) {
        pr_err("xorapi_test: encrypt failed: %d\n", err);
        goto out;
    }

    sg_init_one(&sg[0], ciphertext, sizeof(plaintext));
    sg_init_one(&sg[1], decrypted, sizeof(plaintext));
    skcipher_request_set_crypt(req, &sg[0], &sg[1], sizeof(plaintext), NULL);

    err = crypto_skcipher_decrypt(req);
    if (err) {
        pr_err("xorapi_test: decrypt failed: %d\n", err);
        goto out;
    }

    pr_info("xorapi_test: plaintext = %s\n", plaintext);
    print_hex_dump(KERN_INFO, "xorapi_test: cipher    = ", DUMP_PREFIX_NONE, 16, 1, ciphertext, sizeof(plaintext), false);
    pr_info("xorapi_test: decrypted = %s\n", decrypted);
    pr_info("xorapi_test: roundtrip = %s\n",
        memcmp(plaintext, decrypted, sizeof(plaintext)) == 0 ? "OK" : "FAILED");

    err = memcmp(plaintext, decrypted, sizeof(plaintext)) == 0 ? 0 : -EINVAL;

out:
    kfree(decrypted);
    kfree(ciphertext);
    kfree(src);
    kfree(sg);
    kfree(req);
    crypto_free_skcipher(tfm);
    return err;
}

static void __exit xorapi_test_exit(void) {
    pr_info("xorapi_test: unloaded\n");
}

module_init(xorapi_test_init);
module_exit(xorapi_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test caller for the xorcipher Crypto API skcipher");
