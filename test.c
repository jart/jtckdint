// Copyright 2023 Justine Alexandra Roberts Tunney
//
// Permission to use, copy, modify, and/or distribute this software for
// any purpose with or without fee is hereby granted, provided that the
// above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "jtckdint.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#define TBIT(T) (sizeof(T) * 8 - 1)
#define TMIN(T) (((T) ~(T)0) > 1 ? (T)0 : (T)((__ckd_uintmax_t)1 << TBIT(T)))
#define TMAX(T) (((T) ~(T)0) > 1 ? (T) ~(T)0 : (T)(((__ckd_uintmax_t)1 << TBIT(T)) - 1))

#ifdef __ckd_have_int128
typedef signed __int128 int128_t;
typedef unsigned __int128 uint128_t;
#endif

#define DECLARE_TEST_VECTORS(T)                 \
  static const T k##T[] = {                     \
    0, 1, 2, 3, 4, 5, 6,                        \
    (T)-1, (T)-2, (T)-3, (T)-4, (T)-5, (T)-6,   \
    TMIN(T),                                    \
    (T)(TMIN(T) + 1),                           \
    (T)(TMIN(T) + 2),                           \
    (T)(TMIN(T) + 3),                           \
    (T)(TMIN(T) + 4),                           \
    TMAX(T),                                    \
    (T)(TMAX(T) - 1),                           \
    (T)(TMAX(T) - 2),                           \
    (T)(TMAX(T) - 3),                           \
    (T)(TMAX(T) - 4),                           \
    (T)(TMIN(T) / 2),                           \
    (T)(TMIN(T) / 2 + 1),                       \
    (T)(TMIN(T) / 2 + 2),                       \
    (T)(TMIN(T) / 2 + 3),                       \
    (T)(TMIN(T) / 2 + 4),                       \
    (T)(TMAX(T) / 2),                           \
    (T)(TMAX(T) / 2 - 1),                       \
    (T)(TMAX(T) / 2 - 2),                       \
    (T)(TMAX(T) / 2 - 3),                       \
    (T)(TMAX(T) / 2 - 4),                       \
  }

DECLARE_TEST_VECTORS(int8_t);
DECLARE_TEST_VECTORS(uint8_t);
DECLARE_TEST_VECTORS(int16_t);
DECLARE_TEST_VECTORS(uint16_t);
DECLARE_TEST_VECTORS(int32_t);
DECLARE_TEST_VECTORS(uint32_t);
DECLARE_TEST_VECTORS(int64_t);
DECLARE_TEST_VECTORS(uint64_t);
#ifdef __ckd_have_int128
DECLARE_TEST_VECTORS(int128_t);
DECLARE_TEST_VECTORS(uint128_t);
#endif

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

#define M(T,U,V)                                                        \
  for (unsigned i = 0; i < sizeof(k##U) / sizeof(k##U[0]); ++i) {       \
    U x = k##U[i];                                                      \
    for (unsigned j = 0; j < sizeof(k##V) / sizeof(k##V[0]); ++j) {     \
      T z1, z2;                                                         \
      V y = k##V[j];                                                    \
      assert(ckd_add(&z1, x, y) == __builtin_add_overflow(x, y, &z2));  \
      assert(z1 == z2);                                                 \
      assert(ckd_sub(&z1, x, y) == __builtin_sub_overflow(x, y, &z2));  \
      assert(z1 == z2);                                                 \
      assert(ckd_mul(&z1, x, y) == __builtin_mul_overflow(x, y, &z2));  \
      assert(z1 == z2);                                                 \
    }                                                                   \
  }

#ifdef __ckd_have_int128
#define MM(T, U)       \
  M(T, U, uint8_t)     \
  M(T, U, uint16_t)    \
  M(T, U, uint32_t)    \
  M(T, U, uint64_t)    \
  M(T, U, uint128_t)   \
  M(T, U, int8_t)      \
  M(T, U, int16_t)     \
  M(T, U, int32_t)     \
  M(T, U, int64_t)     \
  M(T, U, int128_t)
#define MMM(T)        \
  MM(T, uint8_t)      \
  MM(T, uint16_t)     \
  MM(T, uint32_t)     \
  MM(T, uint64_t)     \
  MM(T, uint128_t)    \
  MM(T, int8_t)       \
  MM(T, int16_t)      \
  MM(T, int32_t)      \
  MM(T, int64_t)      \
  MM(T, int128_t)
  MMM(uint8_t)
  MMM(uint16_t)
  MMM(uint32_t)
  MMM(uint64_t)
  MMM(uint128_t)
  MMM(int8_t)
  MMM(int16_t)
  MMM(int32_t)
  MMM(int64_t)
  MMM(int128_t)

#else
#define MM(T, U)       \
  M(T, U, uint8_t)     \
  M(T, U, uint16_t)    \
  M(T, U, uint32_t)    \
  M(T, U, uint64_t)    \
  M(T, U, int8_t)      \
  M(T, U, int16_t)     \
  M(T, U, int32_t)     \
  M(T, U, int64_t)
#define MMM(T)        \
  MM(T, uint8_t)      \
  MM(T, uint16_t)     \
  MM(T, uint32_t)     \
  MM(T, uint64_t)     \
  MM(T, int8_t)       \
  MM(T, int16_t)      \
  MM(T, int32_t)      \
  MM(T, int64_t)
  MMM(uint8_t)
  MMM(uint16_t)
  MMM(uint32_t)
  MMM(uint64_t)
  MMM(int8_t)
  MMM(int16_t)
  MMM(int32_t)
  MMM(int64_t)
#endif

}
