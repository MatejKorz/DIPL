/*
    MIT license
    Copyright (C) 2025 Intel Corporation
    Copyright (C) 2026 Matej Korž
    SPDX-License-Identifier: MIT
*/

#ifndef ZEROIZE_H_
#define ZEROIZE_H_

#include <linux/string.h>

/// @brief Zeroizes a buffer using OS-specific functions guaranteed to avoid being optimized out by the compiler.
/// @param[in] secret Incoming buffer to zeroize.
/// @param secret_len Incoming buffer length (in bytes).
void zeroize_secret(void* secret, size_t secret_len);

#endif /* ZEROIZE_H_ */
