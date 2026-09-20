# Sources

## ddd-AES

- [XTS mode revisited: high hopes for key scope](https://arxiv.org/abs/2502.18631)
- [Deck-based wide block cipher modes and an exposition of the blinded keyed hashing model](https://philosophymindscience.org/index.php/ToSC/article/download/8451/8021)
- [Efficient instances of docked double decker with AES, and application to authenticated encryption](https://eprint.iacr.org/2024/084.pdf)

## Modules

- [Building modules](https://docs.kernel.org/kbuild/modules.html)
- [Modprobe man page](https://manpages.debian.org/trixie/kmod/modprobe.8.en.html)
- [Linux module example](https://sysprog21.github.io/lkmpg/#hello-and-goodbye)

### Crypto API

- [Kernel Crypto API architecture](https://www.kernel.org/doc/html/latest/crypto/architecture.html) — transform types, registration model
- [Developing Cipher Algorithms](https://docs.kernel.org/crypto/devel-algos.html) — `crypto_alg`, register/unregister functions, walkthrough
- [Kernel Crypto API index](https://www.kernel.org/doc/html/latest/crypto/index.html)
- [Symmetric Key Cipher API (skcipher)](https://www.kernel.org/doc/html/latest/crypto/api-skcipher.html)
- [Kernel Crypto API userspace interface (AF_ALG)](https://www.kernel.org/doc/html/latest/crypto/userspace-if.html)

### Crypto library

- [libcrypto docs](https://docs.kernel.org/crypto/libcrypto.html)

### History

- [Progress in modernizing kernel cryptography](https://lwn.net/Articles/1077427/)

### Diff

| Property | Tradition crypto API | Crypto library |
|----------|----------------------|----------------|
|---|---|---|
| **Dispatch mechanism** | Runtime name-based lookup (`crypto_alloc_*("cbc(aes)", ...)`) against a registry | Direct compile-time function calls, no lookup |
| **Registration** | `crypto_register_*()` / `crypto_unregister_*()` into a global registry | None — module load just runs init code (e.g. picks best CPU impl) |
| **Visibility** | Shows up in `/proc/crypto` | Not visible anywhere at runtime |
| **API shape** | Struct-of-callbacks (`.setkey`, `.encrypt`, `.decrypt`, etc.) wrapped in `crypto_alg` | Plain named functions (e.g. `sha256_update()`) |
| **Userspace access** | Reachable via `AF_ALG` sockets | Kernel-internal only |
| **Overhead** | Indirect calls, allocation, scatterlist/request machinery | Direct call, no allocation — faster, especially for short inputs |
| **Algorithm agility** | Best implementation chosen at runtime by priority among registered drivers sharing a `cra_name` | Best CPU-specific implementation chosen once at module init; caller fixed to a specific function at compile time |
| **Directory convention** | `crypto/`, `arch/$(SRCARCH)/crypto/`, `drivers/crypto/` | `lib/crypto/`, `lib/crypto/$(SRCARCH)/` |
| **Kconfig style** | Often user-visible tristate options | Hidden tristate `CRYPTO_LIB_<ALGO>`, `select`ed by dependents |
| **Relationship** | — | Where possible, `crypto/` wraps `lib/crypto/` implementations rather than duplicating logic |
| **Best suited for** | Algorithm decided at runtime (userspace-chosen, or hardware-offload drivers plugging in) | Caller already knows exactly which algorithm it wants |
| **Step 1: Implement** | Fill in an alg struct (`skcipher_alg`, `shash_alg`, `aead_alg`, etc.) with callbacks (`.setkey`, `.encrypt`/`.decrypt` or `.init`/`.update`/`.final`) | Write plain dedicated functions in `lib/crypto/<algo>.c` |
| **Step 2: Metadata/headers** | Fill in embedded `struct crypto_alg`: `cra_name`, `cra_driver_name`, `cra_priority`, `cra_blocksize`, `cra_ctxsize`, `cra_module` | Add public API/prototypes to `include/crypto/<algo>.h` |
| **Step 3: Arch optimization (optional)** | Separate driver module in `drivers/crypto/` or `arch/$(SRCARCH)/crypto/`, registered under same `cra_name` with higher priority | Arch-optimized variant under `lib/crypto/$(SRCARCH)/`, often as a header included by the generic `.c` so both build into one module |
| **Step 4: Register/wire-up** | Call `crypto_register_alg()` (or type-specific variant, e.g. `crypto_register_skcipher()`) in `module_init()`; unregister symmetrically in `module_exit()` | No register call needed; init-time logic (module init/initcall) picks best implementation for the running CPU once |
| **Step 5: Kconfig** | Add tristate `CRYPTO_<NAME>` in `crypto/Kconfig` (or `drivers/crypto/Kconfig`), selecting the base transform type needed | Add hidden tristate `CRYPTO_LIB_<ALGO>` in `lib/crypto/Kconfig` (+ `CRYPTO_ARCH_MAY_HAVE_LIB_<ALGO>` if arch-optimized) |
| **Step 6: Makefile** | Add source to `crypto/Makefile` (or driver's Makefile) | Add source to `lib/crypto/Makefile` |
| **Step 7: Testing** | Self-test/`cryptomgr`, or exercise via `crypto_alloc_*()` in-kernel or `AF_ALG`/`kcapi` from userspace | Add KUnit tests under `lib/crypto/tests/` |
| **Step 8: Result** | Algorithm becomes visible in `/proc/crypto`, reachable by name via `crypto_alloc_*("name", ...)` | Callers `#include` the header and call functions directly — no registry, no `/proc/crypto` entry |


## Measuring performance

- [Cryptsetup benchmark](https://man7.org/linux/man-pages/man8/cryptsetup-benchmark.8.html)
- [kcapi-speed](https://github.com/smuellerDD/libkcapi)
- [FIO](https://fio.readthedocs.io/en/latest/)

## OpenSSL

- [Provider docs](https://docs.openssl.org/4.0/man7/provider/)
- [AES OpenSSL provider](https://github.com/openssl/openssl/blob/master/providers/implementations/ciphers/cipher_aes.h)

