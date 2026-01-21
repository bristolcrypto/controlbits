#ifndef PORTABLE_MACROS
#define PORTABLE_MACROS

#include <stdint.h>

//#define crypto_int32 int32_t
//#define int16 int16_t
//#define int32 int32_t
//#define int32_min crypto_int32_min
//#define crypto_sort_int32 int32_sort


#define int32_MINMAX(a,b) \
do { \
  int32_t temp1; \
  asm( \
    "cmpl %1,%0\n\t" \
    "mov %0,%2\n\t" \
    "cmovg %1,%0\n\t" \
    "cmovg %2,%1\n\t" \
    : "+r"(a), "+r"(b), "=r"(temp1) \
    : \
    : "cc" \
  ); \
} while(0)


/* Applies control bit to int16s. */
#define APPLY_CONTROL_BIT(a,b, cb) \
do { \
  int16_t d = a ^ b; \
  cb=-cb; \
  d&=cb; \
  a^=d; \
  b^=d; \
} while(0)


#endif