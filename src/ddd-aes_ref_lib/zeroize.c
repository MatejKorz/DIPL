/*
    MIT license
    Copyright (C) 2025 Intel Corporation
    Copyright (C) 2026 Matej Korž
    SPDX-License-Identifier: MIT
*/

#include "zeroize.h"

void zeroize_secret(void* secret, size_t slen)
{
#ifdef ZEROIZE_SECRETS
    memzero_explicit(secret, slen);
#endif
}
