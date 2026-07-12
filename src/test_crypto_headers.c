#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>

static int __init test_crypto_init(void) {
    pr_info("Crypto Test: Init\n");

    if (crypto_has_alg("cbc(aes)", 0, 0)) {
        pr_info("Crypto Test: AES found\n");
    } else {
        pr_info("Crypto Test: AES not found\n");
    }

    return 0;
}

static void __exit test_crypto_exit(void) {
    pr_info("Crypto Test: Exit\n");
}

module_init(test_crypto_init);
module_exit(test_crypto_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test");
MODULE_DESCRIPTION("Test to check Linux Crypto API headers");
