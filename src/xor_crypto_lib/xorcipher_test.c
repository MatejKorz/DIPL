// SPDX-License-Identifier: GPL-2.0
/*
 * KUnit tests for the lib/crypto-style xor_crypt_ecb().
 *
 * Built in-tree only (wired into lib/crypto/ by kunit.patch): the suite
 * runs either in a UML kernel via tools/testing/kunit/kunit.py, or as
 * the xorcipher_test module on a kernel with CONFIG_KUNIT.
 */

#include <kunit/test.h>
#include <linux/string.h>

#include "xorcipher.h"

static const u8 key[] = "mysecretkey";
static const u8 plaintext[] = "Hello, lib/crypto world!";
/* verified against the old dmesg test on the VM (25 bytes, NUL included) */
static const u8 ciphertext[] = {
	0x25, 0x1c, 0x1f, 0x09, 0x0c, 0x5e, 0x45, 0x18, 0x02, 0x07,
	0x56, 0x0e, 0x0b, 0x0a, 0x15, 0x17, 0x1d, 0x45, 0x03, 0x04,
	0x17, 0x15, 0x09, 0x58, 0x73,
};

static void xorcipher_known_answer(struct kunit *test)
{
	u8 out[sizeof(plaintext)];

	xor_crypt_ecb(key, sizeof(key) - 1, plaintext, out, sizeof(out));

	KUNIT_EXPECT_MEMEQ(test, out, ciphertext, sizeof(out));
}

static void xorcipher_roundtrip(struct kunit *test)
{
	u8 ct[sizeof(plaintext)];
	u8 pt[sizeof(plaintext)];

	xor_crypt_ecb(key, sizeof(key) - 1, plaintext, ct, sizeof(ct));
	xor_crypt_ecb(key, sizeof(key) - 1, ct, pt, sizeof(pt));

	KUNIT_EXPECT_MEMEQ(test, pt, plaintext, sizeof(pt));
}

static void xorcipher_inplace(struct kunit *test)
{
	u8 buf[sizeof(plaintext)];

	memcpy(buf, plaintext, sizeof(buf));

	xor_crypt_ecb(key, sizeof(key) - 1, buf, buf, sizeof(buf));
	KUNIT_EXPECT_MEMEQ(test, buf, ciphertext, sizeof(buf));

	xor_crypt_ecb(key, sizeof(key) - 1, buf, buf, sizeof(buf));
	KUNIT_EXPECT_MEMEQ(test, buf, plaintext, sizeof(buf));
}

/* key shorter than the buffer: exercises the i % keylen wraparound */
static void xorcipher_short_key(struct kunit *test)
{
	static const u8 short_key[] = "abc";
	static const u8 src[] = "Hello";
	static const u8 expected[] = { 0x29, 0x07, 0x0f, 0x0d, 0x0d, 0x63 };
	u8 out[sizeof(src)];

	xor_crypt_ecb(short_key, sizeof(short_key) - 1, src, out, sizeof(out));

	KUNIT_EXPECT_MEMEQ(test, out, expected, sizeof(out));
}

static void xorcipher_len_zero_noop(struct kunit *test)
{
	u8 out[1] = { 0xaa };

	xor_crypt_ecb(key, sizeof(key) - 1, plaintext, out, 0);

	KUNIT_EXPECT_EQ(test, out[0], 0xaa);
}

static struct kunit_case xorcipher_test_cases[] = {
	KUNIT_CASE(xorcipher_known_answer),
	KUNIT_CASE(xorcipher_roundtrip),
	KUNIT_CASE(xorcipher_inplace),
	KUNIT_CASE(xorcipher_short_key),
	KUNIT_CASE(xorcipher_len_zero_noop),
	{}
};

static struct kunit_suite xorcipher_test_suite = {
	.name = "xorcipher",
	.test_cases = xorcipher_test_cases,
};

kunit_test_suite(xorcipher_test_suite);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KUnit tests for the lib/crypto XOR cipher");
