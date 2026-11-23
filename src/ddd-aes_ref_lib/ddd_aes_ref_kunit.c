// SPDX-License-Identifier: GPL-2.0
/*
 * KUnit tests for ddd-AES reference
 *
 * Copyright (C) 2026 Matej Korž
 */

#include <kunit/test.h>
#include <linux/string.h>
#include <linux/compiler_attributes.h> // __nonstring

// for now, since tests live in crypto/tests, later replace by <crypto/ddd_aes_ref.h> and fix for out-of-tree builds
#include "../ddd_aes_ref.h"

static const u8 plaintext[2 * BLOCKSIZE_BYTES + 1] __nonstring = "ABCDEFGHIJKLMNOPX0123456701234567";
// static u8 ciphertext[2 * BLOCKSIZE_BYTES + 1];
static const u8 tweak[TWEAKSIZE_BYTES] = {0x05, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

static const u8 key_k[KEYSIZE_BYTES] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
                                0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0};
static const u8 key_l[KEYSIZE_BYTES] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

/// Basic negative input validation tests

static void ref_neg_input(struct kunit *test) {
    int status = ddd_aes_ref_encrypt(NULL, plaintext, sizeof(plaintext), key_k, key_l, tweak, TWEAKSIZE_BYTES);
    KUNIT_EXPECT_LT_MSG(test, status, 0, "TEST FAILED (null ciphertext): status %d", status);
}


static struct kunit_case ddd_aes_ref_test_cases[] = {KUNIT_CASE(ref_neg_input), {}};

static struct kunit_suite ddd_aes_ref_test_suite = {
    .name = "ddd_aes_ref",
    .test_cases = ddd_aes_ref_test_cases,
};

kunit_test_suite(ddd_aes_ref_test_suite);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KUnit tests for ddd-AES reference");
MODULE_AUTHOR("Matej Korž");
