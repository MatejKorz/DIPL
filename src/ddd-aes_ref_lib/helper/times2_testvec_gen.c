#include <emmintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void times_2(__m128i *out) {
    uint64_t buf[2];
    _mm_storeu_si128((__m128i *)buf, *out);

    uint64_t carry_in = 0;
    uint64_t carry_out = 0;
    for (size_t i = 0; i < 2; ++i) {
        carry_out = (buf[i] >> 63);
        buf[i] = (buf[i] << 1) ^ carry_in;
        carry_in = carry_out;
    }

    buf[0] ^= 0x87 & ((~carry_in) + 1);
    *out = _mm_loadu_si128((__m128i *)buf);
}

static int check(uint64_t lo, uint64_t hi, uint64_t elo, uint64_t ehi) {
    uint64_t in[2] = {lo, hi}, res[2];
    __m128i v = _mm_loadu_si128((__m128i *)in);
    times_2(&v);
    _mm_storeu_si128((__m128i *)res, v);
    int ok = res[0] == elo && res[1] == ehi;
    printf("in  = %016llx %016llx\nout = %016llx %016llx  %s\n\n", (unsigned long long)hi, (unsigned long long)lo,
           (unsigned long long)res[1], (unsigned long long)res[0], ok ? "OK" : "MISMATCH");
    return ok;
}

int main(void) {
    int ok = 1;
    ok &= check(1, 0, 2, 0);                                 /* x * 1 = x */
    ok &= check(0x8000000000000000ULL, 0, 0, 1);             /* carry between words */
    ok &= check(0, 0x8000000000000000ULL, 0x87, 0);          /* reduction */
    ok &= check(~0ULL, ~0ULL, 0xFFFFFFFFFFFFFF79ULL, ~0ULL); /* both */
    return !ok;
}
