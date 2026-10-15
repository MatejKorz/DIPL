#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/printk.h>
#include "xorcipher.h"

static int __init xorlib_test_init(void) {
    static const u8 key[] = "mysecretkey";
    static const u8 plaintext[] = "Hello, lib/crypto world!";
    u8 ciphertext[sizeof(plaintext)];
    u8 decrypted[sizeof(plaintext)];

    xor_crypt_ecb(key, sizeof(key) - 1, plaintext, ciphertext, sizeof(plaintext));
    xor_crypt_ecb(key, sizeof(key) - 1, ciphertext, decrypted, sizeof(plaintext));

    pr_info("xorlib_test: plaintext = %s\n", plaintext);
    print_hex_dump(KERN_INFO, "xorlib_test: cipher    = ", DUMP_PREFIX_NONE, 16, 1, ciphertext, sizeof(ciphertext), false);
    pr_info("xorlib_test: decrypted = %s\n", decrypted);
    pr_info("xorlib_test: roundtrip = %s\n",
        memcmp(plaintext, decrypted, sizeof(plaintext)) == 0 ? "OK" : "FAILED");

    return 0;
}

static void __exit xorlib_test_exit(void) {
    pr_info("xorlib_test: unloaded\n");
}

module_init(xorlib_test_init);
module_exit(xorlib_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test caller for the lib/crypto-style xor_crypt_ecb()");
