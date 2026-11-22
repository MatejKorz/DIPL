/*
    MIT license
    Copyright (C) 2025 Intel Corporation
    Copyright (C) 2026 Matej Korž
    SPDX-License-Identifier: MIT
*/

#ifndef DDD_AES_REF_H_
#define DDD_AES_REF_H_

#include <linux/types.h>

// In the default version, we assume |c|=0 and use 28 bits for the counter j, so we can encrypt up to 2**28 - 1 blocks
#define MAXENCRYPTBLOCKS (1 << 28)

// Tweak size in bytes
#define TWEAKSIZE_BYTES 16

int xor_h(u8 *out, const u8 *const in, const size_t len, const u8 *const raw_key);

/// @brief DDD-AES encryption implementation.
/// @param[out] cipher Ciphertext (output). The result of DDD-AES encryption function application to @p plain.
/// @param[in] plain Plaintext (input).
/// @param[in] len Plaintext length (in bytes).
/// @param[in] key_k F_K() function key. Precondition: the key is 128-bit-long.
/// @param[in] key_l H_L() function key. Precondition: the key is 128-bit-long.
/// @param[in] tweak Tweak.
/// @param[in] tweak_len Tweak length (in bytes).
/// @return 0 on success, error code < 0 on error.
int ddd_aes_ref_read_encrypt(u8* cipher,
                             const u8* const plain,
                             const size_t len,
                             const u8* const key_k,
                             const u8* const key_l,
                             const u8* const tweak,
                             const size_t tweak_len);

/// @brief DDD-AES decryption implementation.
/// @param[out] plain Plaintext (output). The result of DDD-AES decryption function application to @p cipher.
/// @param[in] cipher Ciphertext (input).
/// @param[in] len Ciphertext length (in bytes).
/// @param[in] key_k F_K() function key. Precondition: the key is 128-bit-long.
/// @param[in] key_l H_L() function key. Precondition: the key is 128-bit-long.
/// @param[in] tweak Tweak.
/// @param[in] tweak_len Tweak length (in bytes).
/// @return 0 on success, error code < 0 on error.
int ddd_aes_ref_read_decrypt(u8* plain,
                             const u8* const cipher,
                             const size_t len,
                             const u8* const key_k,
                             const u8* const key_l,
                             const u8* const tweak,
                             const size_t tweak_len);

#endif /* DDD_AES_REF_H_ */
