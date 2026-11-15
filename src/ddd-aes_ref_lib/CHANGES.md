# Changes to port to kernel

- zeroize - replace with kernel call `memzero_explicit()`
- polyval - replace with kernel implementation from `<crypto/gf128hash.h>`
- aes - replace with kerlen implementation from `<crypto/aes.h>`
