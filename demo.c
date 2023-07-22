// run ./demo.sh

#include "jtckdint.h"

#ifdef __cplusplus
extern "C" {
#endif

char ckd_add_unsigned_unsigned_unsigned(unsigned long *z, unsigned long x, unsigned long y) {
  return ckd_add(z, x, y);
}
char ckd_add_signed_signed_signed(signed long *z, signed long x, signed long y) {
  return ckd_add(z, x, y);
}
char ckd_add_unsigned_signed_signed(unsigned long *z, signed long x, signed long y) {
  return ckd_add(z, x, y);
}

char ckd_sub_unsigned_unsigned_unsigned(unsigned long *z, unsigned long x, unsigned long y) {
  return ckd_sub(z, x, y);
}
char ckd_sub_signed_signed_signed(signed long *z, signed long x, signed long y) {
  return ckd_sub(z, x, y);
}
char ckd_sub_unsigned_signed_signed(unsigned long *z, signed long x, signed long y) {
  return ckd_sub(z, x, y);
}

char ckd_mul_unsigned_unsigned_unsigned(unsigned long *z, unsigned long x, unsigned long y) {
  return ckd_mul(z, x, y);
}
char ckd_mul_signed_signed_signed(signed long *z, signed long x, signed long y) {
  return ckd_mul(z, x, y);
}
char ckd_mul_unsigned_signed_signed(unsigned long *z, signed long x, signed long y) {
  return ckd_mul(z, x, y);
}

#ifdef __cplusplus
}
#endif
