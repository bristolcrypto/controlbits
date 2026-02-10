#ifndef PORTABLE_MACROS
#define PORTABLE_MACROS

#include <stdint.h>

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