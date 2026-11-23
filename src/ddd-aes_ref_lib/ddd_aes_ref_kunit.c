// SPDX-License-Identifier: GPL-2.0
/*
 * KUnit tests for ddd-AES reference
 *
 * Copyright (C) 2026 Matej Korž
 */

#include <kunit/test.h>
#include <kunit/visibility.h>
#include <linux/compiler_attributes.h> // __nonstring
#include <linux/string.h>
#include <linux/unaligned.h>

// for now, since tests live in crypto/tests, later replace by <crypto/ddd_aes_ref.h> and fix for out-of-tree builds
#include "../ddd_aes_ref.h"

MODULE_IMPORT_NS("EXPORTED_FOR_KUNIT_TESTING");

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

struct times2_case {
    u64 lo, hi;   /* input halves, little-endian */
    u64 elo, ehi; /* expected halves */
    const char *name;
};

static const struct times2_case times2_cases[] = {
    {1, 0, 2, 0, "x_times_one"},
    {0x8000000000000000ULL, 0, 0, 1, "carry_between_words"},
    {0, 0x8000000000000000ULL, 0x87, 0, "reduction"},
    {~0ULL, ~0ULL, 0xFFFFFFFFFFFFFF79ULL, ~0ULL, "carry_and_reduction"},
};
KUNIT_ARRAY_PARAM_DESC(times2, times2_cases, name);

static void internal_times_2(struct kunit *test) {
    const struct times2_case *tc = test->param_value;
    u8 blk[AES_BLOCK_SIZE];
    u8 exp[AES_BLOCK_SIZE];

    put_unaligned_le64(tc->lo, &blk[0]);
    put_unaligned_le64(tc->hi, &blk[8]);
    put_unaligned_le64(tc->elo, &exp[0]);
    put_unaligned_le64(tc->ehi, &exp[8]);

    times_2(blk);

    KUNIT_EXPECT_MEMEQ(test, blk, exp, sizeof(blk));
}

static struct kunit_case internal_test_cases[] = {KUNIT_CASE_PARAM(internal_times_2, times2_gen_params), {}};

static struct kunit_suite ddd_aes_ref_internal_test_suite = {
    .name = "ddd_aes_ref internal",
    .test_cases = internal_test_cases,
};

kunit_test_suites(&ddd_aes_ref_test_suite, &ddd_aes_ref_internal_test_suite);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KUnit tests for ddd-AES reference");
MODULE_AUTHOR("Matej Korž");
