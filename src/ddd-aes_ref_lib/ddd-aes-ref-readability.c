/*
    MIT license
    Copyright (C) 2025 Intel Corporation
    Copyright (C) 2026 Matej Korž
    SPDX-License-Identifier: MIT
*/

#include "ddd-aes-ref-readability.h"

/// @brief DDD-AES H_L() function implementation.
/// @param[out] out Output data. The result of H_L() evaluation on input data is XORed into this buffer.
/// @param[in] in Input data.
/// @param[in] len Input data length (in bytes).
/// @param[in] raw_key H_L() function key. Precondition: the key is 128-bit-long.
/// @return 0 on success, error code < 0 on error.
int xor_h(u8* out, const u8* const in, const size_t len, const u8* const raw_key)
{
	struct polyval_key key;
	struct polyval_ctx ctx;
	u8 tag[POLYVAL_DIGEST_SIZE];
	u8 tmp[POLYVAL_BLOCK_SIZE] = {};
	u8 lenblk[POLYVAL_BLOCK_SIZE] = {};
	const size_t leftover = len % POLYVAL_BLOCK_SIZE;
	size_t len_bits = len * 8;

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
static void times_2(__m128i* out)
{
    // Multiplication by two is a shift by 1 with a conditional XOR with a constant
    uint64_t buf[2];
    _mm_storeu_si128((__m128i*)buf, *out);

    uint64_t carry_in = 0;
    uint64_t carry_out = 0;
    for (size_t i = 0; i < 2; ++i) {
        carry_out = (buf[i] >> 63);
        buf[i] = (buf[i] << 1) ^ carry_in;
        carry_in = carry_out;
    }

    buf[0] ^= 0x87 & ((~carry_in) + 1);
    *out = _mm_loadu_si128((__m128i*)buf);

    memzero_explicit(buf, sizeof(buf));
}

/// @brief DDD-AES F_K() function implementation. XORs to the existing value of @p out the result of the PRF
/// @brief evaluation on @p in.
/// @param[out] out Output data. The result of F_K() evaluation on input data is XORed into this buffer.
/// @param[in] outlen Size of the output in bytes.
/// @param[in] in Input data block.
/// @param[in] key_k 128-bit F_K() function key.
/// @param[in] b Domain separator value B.
/// @param[in] tweak Tweak value W.
/// @param[in] tweak_len length of Tweak value W.
void xor_f(uint8_t* out,
           const size_t outlen,
           const uint8_t in[BLOCKSIZE_BYTES],
           const uint8_t* const key_k,
           const uint8_t b,
           const uint8_t tweak[TWEAKSIZE_BYTES])
{
    const size_t num_full_blocks = outlen / BLOCKSIZE_BYTES; // Number of full AES blocks
    const size_t last_block_len = outlen - num_full_blocks * BLOCKSIZE_BYTES;

    // Prepare expanded key
    uint8_t rnd_keys[RNDKEYSIZE_BYTES];
    aes_key_sched(rnd_keys, key_k);

    // Prepare the W||B value:
    uint64_t w_b[2] = { 0ULL, 0ULL };
    // Load tweak value
    for (size_t i = 0; i < 8; ++i) {
        w_b[0] |= (uint64_t)tweak[i] << (8 * i);
    }
    // Shift left to make space for B and fill in B
    w_b[0] = (w_b[0] << 4) | (b & 0x0F);
    for (size_t i = 0; i < 8; ++i) {
        w_b[1] |= (uint64_t)tweak[8 + i] << (8 * i);
    }
    w_b[1] = w_b[1] << 4;
    // Move 4 MSB of tweak[7] to the second quadword of tweak
    w_b[1] |= ((tweak[7]) & 0xF0) >> 4;

    __m128i s = _mm_loadu_si128((__m128i*)w_b);
    s = aes_enc(s, rnd_keys);
    __m128i i = _mm_loadu_si128((__m128i*)in);

    uint8_t* memory_block = out;
    for (size_t idx = 0; idx < num_full_blocks; ++idx) {
        // Generate keystream
        __m128i z_j = aes_enc(_mm_xor_si128(i, s), rnd_keys);
        _mm_storeu_si128((__m128i*)memory_block, _mm_xor_si128(z_j, _mm_loadu_si128((__m128i*)memory_block)));
        memory_block += BLOCKSIZE_BYTES;
        // Multiply mask by two
        times_2(&s);

        memzero_explicit(&z_j, sizeof(z_j));
    }

    // Handle the possible last incomplete block
    if (last_block_len != 0) {
        uint8_t last_z[BLOCKSIZE_BYTES];
        __m128i z_j = aes_enc(_mm_xor_si128(i, s), rnd_keys);
        _mm_storeu_si128((__m128i*)last_z, z_j);
        for (size_t i = 0; i < last_block_len; ++i) {
            *memory_block++ ^= last_z[i];
        }

        memzero_explicit(&last_z, sizeof(last_z));
        memzero_explicit(&z_j, sizeof(z_j));
    }

    memzero_explicit(rnd_keys, sizeof(rnd_keys));
    memzero_explicit(&s, sizeof(s));
    memzero_explicit(&i, sizeof(i));
}

int ddd_aes_ref_read_encrypt(uint8_t* cipher,
                             const uint8_t* const plain,
                             const size_t len,
                             const uint8_t* const key_k,
                             const uint8_t* const key_l,
                             const uint8_t* const tweak,
                             const size_t tweak_len)
{
    // Basic input validation
    if (plain == NULL || cipher == NULL || key_k == NULL || key_l == NULL || tweak == NULL) {
        return -1;
    }

    // Plaintext length must be at least two blocks and at most the number of blocks supported by the scheme.
    if (len > (MAXENCRYPTBLOCKS - 1) * (size_t)BLOCKSIZE_BYTES || len < 2 * BLOCKSIZE_BYTES) {
        return -1;
    }

    // Tweak length must be TWEAKSIZE
    if (tweak_len != TWEAKSIZE_BYTES) {
        return -1;
    }

    memcpy(cipher, plain, len);

    uint8_t* t = cipher;
    uint8_t* u = cipher + BLOCKSIZE_BYTES;
    uint8_t* v = cipher + len - BLOCKSIZE_BYTES;

    int status = xor_h(t, u, len - BLOCKSIZE_BYTES, key_l);
    if (status < 0) {
        printf("ERROR: xor_h() failed, status: %d\n", status);
        return status;
    }

    xor_f(v, BLOCKSIZE_BYTES, t, key_k, 0x1, tweak);
    xor_f(t, len - BLOCKSIZE_BYTES, v, key_k, 0x2, tweak);

    status = xor_h(v, t, len - BLOCKSIZE_BYTES, key_l);
    if (status < 0) {
        printf("ERROR: xor_h() failed, status: %d\n", status);
        return status;
    }

    return 0;
}

int ddd_aes_ref_read_decrypt(uint8_t* plain,
                             const uint8_t* const cipher,
                             const size_t len,
                             const uint8_t* const key_k,
                             const uint8_t* const key_l,
                             const uint8_t* const tweak,
                             const size_t tweak_len)
{
    // Basic input validation
    if (plain == NULL || cipher == NULL || key_k == NULL || key_l == NULL || tweak == NULL) {
        return -1;
    }

    // Ciphertext length must be at least two blocks and at most the number of blocks supported by the scheme.
    if (len > (MAXENCRYPTBLOCKS - 1) * (size_t)BLOCKSIZE_BYTES || len < 2 * BLOCKSIZE_BYTES) {
        return -1;
    }

    // Tweak length must be TWEAKSIZE
    if (tweak_len != TWEAKSIZE_BYTES) {
        return -1;
    }

    memcpy(plain, cipher, len);

    uint8_t* t = plain;
    uint8_t* u = plain + BLOCKSIZE_BYTES;
    uint8_t* v = plain + len - BLOCKSIZE_BYTES;

    int status = xor_h(v, t, len - BLOCKSIZE_BYTES, key_l);
    if (status < 0) {
        printf("ERROR: xor_h() failed, status: %d\n", status);
        return status;
    }

    xor_f(t, len - BLOCKSIZE_BYTES, v, key_k, 0x2, tweak);
    xor_f(v, BLOCKSIZE_BYTES, t, key_k, 0x1, tweak);

    status = xor_h(t, u, len - BLOCKSIZE_BYTES, key_l);
    if (status < 0) {
        printf("ERROR: xor_h() failed, status: %d\n", status);
        return status;
    }

    return 0;
}
