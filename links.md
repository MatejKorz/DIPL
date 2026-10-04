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

Linux Crypto API is the original option for using cryptographic algorithms in the kernel. It provides access to ciphers (or transformations, which is how documentation calls them), hashes, random number generation, and more. The API is a registry of these transformations, that kernel code can request by name, instead of calling the implementation directly. Transformations such as `struct crypto_alg` are registered and removed by calling `crypto_register_alg()` and `crypto_unregister_alg()` respectively. Each specific transformation type has its own API and structures to register and functions calls to use, for example `struct ahash_alg` for hashes. Registered structures contain name, cipher parameters and priority, for different driver implementations.

To use Crypto API in kernel, transform handle is found by its name and is allocated. Then input parameters such as key, initialization vector and scatterlists for data buffer are set and encrypt or decrypt can be called. The request can be completed synchronously or via callback, based on implementation and set flags.

The Crypto API can be also exposed to userspace by AF_ALG socket. Main advantage of the API is the flexibility at runtime. Algorithm can be chosen from userspace by string name without any knowledge at compile time. This allows tools such as dm-crypt/cryptsetup to use newer algorithms without any internal changes. The best implementation can be also picked by the priority setup in the transformation structure. It support asynchronous requests, so cryptographic accelerators can be used.

But this also ties to the disadvantages of this API. Due to each call being executed by function pointers, it cannot be optimized by compiler in runtime. Also there is a substantial overhead in transformation lookup, and initialization, especially for smaller or one-off operations. Use for the API is complex and verbose, including handle and request allocation, scatterlist building and setting parameters and callbacks and then freeing everything.

### Crypto library

- [libcrypto docs](https://docs.kernel.org/crypto/libcrypto.html)
- [KUnit tests](https://docs.kernel.org/dev-tools/kunit/index.html)

Linux kernel's crypto library, located in `lib/crypto` is a newer alternative to the traditional crypto API. Main benefit over the aforementioned crypto API, is faster and easily accessible algorithms. Code is run without indirect calls, memory allocation, lookups in algorithm registry and used by the API. Library functions are linked statically, instead of dynamic loading by algorithm name.

Code is split between:
- `lib/crypto/*.c` - implementation
- `lib/crypto/$(SRCARCH)/` - specific optimizations for current architecture
- `include/crypto/` - header files

Testing of library functions is also standardized to use KUnit tests, located in `lib/crypto/tests`. ...

Recommendation from the kernel developers, at least according to the official documentations is to use this approach when implementing new algorithms or using exiting ones in the in-kernel code. Reasons for this are multiple. One of the most important is performance, as was mentioned earlier. Another reason is simplicity of the code. Exception for this may be the backwards compatibility with existing tools like cryptsetup and dm-crypt. Mainly these programs are used for disk encryption on Linux systems and they allow users to select the algorithm by input string, to use for the encryption. For this use-case is necessary to use the API, otherwise tools itself would need to support and include in their code calls to specific library function on each of the existing, or rather supported architectures.

This issue can be solved, by implementing algorithm in crypto library, but writing a short wrapper around it, to be able to register it in the traditional API. This approach will bring best of the both worlds, speed and simplicity of the library functions if it is needed, but also the compatibility and easy integration with existing tools, by being able to access it from the API. This approach is also suggested by the official documentation.

### History

- [Progress in modernizing kernel cryptography](https://lwn.net/Articles/1077427/)

## Measuring performance

- [Cryptsetup benchmark](https://man7.org/linux/man-pages/man8/cryptsetup-benchmark.8.html)
- [kcapi-speed](https://github.com/smuellerDD/libkcapi)
- [FIO](https://fio.readthedocs.io/en/latest/)

## OpenSSL

- [Provider docs](https://docs.openssl.org/4.0/man7/provider/)
- [AES OpenSSL provider](https://github.com/openssl/openssl/blob/master/providers/implementations/ciphers/cipher_aes.h)

