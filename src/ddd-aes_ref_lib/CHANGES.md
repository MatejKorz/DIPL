# Changes to port to kernel

- zeroize - replace with kernel call `memzero_explicit()`
    - maps 1:1 to memzero_explicit, just replace all calls
- polyval - replace with kernel implementation from `<crypto/gf128hash.h>`
    - file dropped, use function from kernel straight in reference implementation file
- aes - replace with kerlen implementation from `<crypto/aes.h>`
    - aes_key_sched -> aes_prepareenckey
    - aes_enc -> aes_encrypt
    - rnd_keys[RNDKEYSIZE_BYTES] -> struct aes_enckey / aes_encrypt_arg union
    - KEYSIZE_BYTES -> AES_KEYSIZE_128
- replace the fpu calls in reference implementation
- times2 function implementation from the `<crypto/gf128mul.h>`

## Changes from example with xor

- tests in `crypto/tests`
- dependency on other modules (CRYPTO_LIB_AES, CRYPTO_LIB_GF128HASH)
- testing static function with kunit [here](https://www.kernel.org/doc/html/latest/dev-tools/kunit/usage.html#testing-static-functions)
