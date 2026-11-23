/*
    MIT license
    Copyright (C) 2025 Intel Corporation
    Copyright (C) 2026 Matej Korž
    SPDX-License-Identifier: MIT
*/

#include <linux/module.h>
#include <linux/export.h>
#include <linux/string.h>
#include <linux/unaligned.h>

#include <crypto/aes.h>
#include <crypto/gf128hash.h>
#include <crypto/gf128mul.h>

#include "ddd_aes_ref.h"

/// @brief DDD-AES H_L() function implementation.
/// @param[out] out Output data. The result of H_L() evaluation on input data is XORed into this buffer.
/// @param[in] in Input data.
/// @param[in] len Input data length (in bytes).
/// @param[in] raw_key H_L() function key. Precondition: the key is 128-bit-long.
/// @return 0 on success, error code < 0 on error.
int xor_h(u8 *out, const u8 *const in, const size_t len, const u8 *const raw_key) {
    struct polyval_key key;
    struct polyval_ctx ctx;
    u8 tag[POLYVAL_DIGEST_SIZE];
    u8 tmp[POLYVAL_BLOCK_SIZE] = {};
    u8 lenblk[POLYVAL_BLOCK_SIZE] = {};
    const size_t leftover = len % POLYVAL_BLOCK_SIZE;
    u64 len_bits = (u64)len * 8;

    polyval_preparekey(&key, raw_key);
    polyval_init(&ctx, &key);

    polyval_update(&ctx, in, len - leftover);
    if (leftover) {
        memcpy(tmp, in + len - leftover, leftover);
        polyval_update(&ctx, tmp, POLYVAL_BLOCK_SIZE);
        memzero_explicit(tmp, sizeof(tmp));
    }
    memcpy(lenblk, &len_bits, sizeof(len_bits));
    polyval_update(&ctx, lenblk, POLYVAL_BLOCK_SIZE);
    polyval_final(&ctx, tag);

    for (size_t i = 0; i < POLYVAL_DIGEST_SIZE; ++i)
        out[i] ^= tag[i];

    memzero_explicit(tag, sizeof(tag));
    memzero_explicit(lenblk, sizeof(lenblk));
    memzero_explicit(&key, sizeof(key));
    return 0;
}

/// @brief Finite-field multiplication by a constant 2.
/// @brief Same field as in AES-XTS.
/// @param[in,out] out Input/output data. out = 2 * out. Precondition: must be exactly 128 bits.
static void times_2(u8 out[AES_BLOCK_SIZE]) {
    le128 t;

	memcpy(&t, out, sizeof(t));
	gf128mul_x_ble(&t, &t);
	memcpy(out, &t, sizeof(t));

    memzero_explicit(&t, sizeof(t));
}

/// @brief DDD-AES F_K() function implementation. XORs to the existing value of @p out the result of the PRF
/// @brief evaluation on @p in.
/// @param[out] out Output data. The result of F_K() evaluation on input data is XORed into this buffer.
/// @param[in] outlen Size of the output in bytes.
/// @param[in] in Input data block.
/// @param[in] key_k 128-bit F_K() function key.
/// @param[in] b Domain separator value B.
/// @param[in] tweak Tweak value W.
static int xor_f(u8 *out, const size_t outlen, const u8 in[AES_BLOCK_SIZE], const u8 *key_k, const u8 b,
          const u8 tweak[TWEAKSIZE_BYTES]) {
    struct aes_enckey enc;
    u8 s[AES_BLOCK_SIZE];     /* mask: AES_K(W || B), then 2*s per block */
    u8 i_blk[AES_BLOCK_SIZE]; /* private copy of "in" */
    u8 z[AES_BLOCK_SIZE];     /* keystream block */
    const size_t num_full_blocks = outlen / AES_BLOCK_SIZE;
    const size_t last_block_len = outlen - num_full_blocks * AES_BLOCK_SIZE;
    u8 *memory_block = out;
    u64 w_b[2] = {0, 0};

    /* Prepare expanded key (encryption only) */
    if (aes_prepareenckey(&enc, key_k, AES_KEYSIZE_128)) return -EINVAL;

    /* Prepare the W||B value: 60 bits of W, then B in the low nibble,
     * with the 4 MSBs of tweak[7] moved into the high quadword. */
    for (size_t idx = 0; idx < 8; ++idx)
        w_b[0] |= (u64)tweak[idx] << (8 * idx);
    w_b[0] = (w_b[0] << 4) | (b & 0x0F);
    for (size_t idx = 0; idx < 8; ++idx)
        w_b[1] |= (u64)tweak[8 + idx] << (8 * idx);
    w_b[1] = (w_b[1] << 4) | ((u64)(tweak[7] & 0xF0) >> 4);

    put_unaligned_le64(w_b[0], &s[0]);
    put_unaligned_le64(w_b[1], &s[8]);
    memzero_explicit(w_b, sizeof(w_b));

    aes_encrypt(&enc, s, s); /* s = AES_K(W || B) */
    memcpy(i_blk, in, AES_BLOCK_SIZE);

    for (size_t idx = 0; idx < num_full_blocks; ++idx) {
        /* z = AES_K(in ^ s) */
        for (size_t j = 0; j < AES_BLOCK_SIZE; ++j)
            z[j] = i_blk[j] ^ s[j];
        aes_encrypt(&enc, z, z);

        for (size_t j = 0; j < AES_BLOCK_SIZE; ++j)
            memory_block[j] ^= z[j];

        memory_block += AES_BLOCK_SIZE;
        times_2(s); /* multiply mask by two */
        memzero_explicit(z, sizeof(z));
    }

    /* Handle the possible last incomplete block (mask is not advanced) */
    if (last_block_len) {
        for (size_t j = 0; j < AES_BLOCK_SIZE; ++j)
            z[j] = i_blk[j] ^ s[j];
        aes_encrypt(&enc, z, z);

        for (size_t j = 0; j < last_block_len; ++j)
            memory_block[j] ^= z[j];

        memzero_explicit(z, sizeof(z));
    }

    memzero_explicit(s, sizeof(s));
    memzero_explicit(i_blk, sizeof(i_blk));
    memzero_explicit(&enc, sizeof(enc));

    return 0;
}

int ddd_aes_ref_encrypt(u8 *cipher, const u8 *const plain, const size_t len, const u8 *const key_k,
                             const u8 *const key_l, const u8 *const tweak, const size_t tweak_len) {
    // Basic input validation
    if (plain == NULL || cipher == NULL || key_k == NULL || key_l == NULL || tweak == NULL) { return -EINVAL; }

    // Plaintext length must be at least two blocks and at most the number of blocks supported by the scheme.
    if (len > (MAXENCRYPTBLOCKS - 1) * (size_t)AES_BLOCK_SIZE || len < 2 * AES_BLOCK_SIZE) { return -EINVAL; }

    // Tweak length must be TWEAKSIZE
    if (tweak_len != TWEAKSIZE_BYTES) { return -EINVAL; }

    memcpy(cipher, plain, len);

    u8 *t = cipher;
    u8 *u = cipher + AES_BLOCK_SIZE;
    u8 *v = cipher + len - AES_BLOCK_SIZE;

    int status = xor_h(t, u, len - AES_BLOCK_SIZE, key_l);
    if (status < 0) {
        pr_err("ERROR: xor_h() failed, status %d:\n", status);
        return status;
    }

    status = xor_f(v, AES_BLOCK_SIZE, t, key_k, 0x1, tweak);
    if (status < 0) {
        pr_err("ERROR: xor_f() failed, status %d:\n", status);
        return status;
    }
    status = xor_f(t, len - AES_BLOCK_SIZE, v, key_k, 0x2, tweak);
    if (status < 0) {
        pr_err("ERROR: xor_f() failed, status %d:\n", status);
        return status;
    }

    status = xor_h(v, t, len - AES_BLOCK_SIZE, key_l);
    if (status < 0) {
        pr_err("ERROR: xor_h() failed, status %d:\n", status);
        return status;
    }

    return 0;
}

int ddd_aes_ref_decrypt(u8 *plain, const u8 *const cipher, const size_t len, const u8 *const key_k,
                             const u8 *const key_l, const u8 *const tweak, const size_t tweak_len) {
    // Basic input validation
    if (plain == NULL || cipher == NULL || key_k == NULL || key_l == NULL || tweak == NULL) { return -EINVAL; }

    // Ciphertext length must be at least two blocks and at most the number of blocks supported by the scheme.
    if (len > (MAXENCRYPTBLOCKS - 1) * (size_t)AES_BLOCK_SIZE || len < 2 * AES_BLOCK_SIZE) { return -EINVAL; }

    // Tweak length must be TWEAKSIZE
    if (tweak_len != TWEAKSIZE_BYTES) { return -EINVAL; }

    memcpy(plain, cipher, len);

    u8 *t = plain;
    u8 *u = plain + AES_BLOCK_SIZE;
    u8 *v = plain + len - AES_BLOCK_SIZE;

    int status = xor_h(v, t, len - AES_BLOCK_SIZE, key_l);
    if (status < 0) {
        pr_err("ERROR: xor_h() failed, status %d:\n", status);
        return status;
    }

    status = xor_f(t, len - AES_BLOCK_SIZE, v, key_k, 0x2, tweak);
    if (status < 0) {
        pr_err("ERROR: xor_f() failed, status %d:\n", status);
        return status;
    }
    status = xor_f(v, AES_BLOCK_SIZE, t, key_k, 0x1, tweak);
    if (status < 0) {
        pr_err("ERROR: xor_f() failed, status %d:\n", status);
        return status;
    }

    status = xor_h(t, u, len - AES_BLOCK_SIZE, key_l);
    if (status < 0) {
        pr_err("ERROR: xor_h() failed, status %d:\n", status);
        return status;
    }

    return 0;
}


EXPORT_SYMBOL_GPL(ddd_aes_ref_encrypt);
EXPORT_SYMBOL_GPL(ddd_aes_ref_decrypt);
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("ddd-AES reference version");
MODULE_AUTHOR("Matej Korž");
