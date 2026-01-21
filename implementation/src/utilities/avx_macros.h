#ifndef AVX_MACROS
#define AVX_MACROS

#include <stdint.h>
#include <immintrin.h>


typedef __m256i int32x8;
typedef __m256i int16x16;
#define int32x8_load(z) _mm256_loadu_si256((__m256i *) (z))
#define int32x8_store(z,i) _mm256_storeu_si256((__m256i *) (z),(i))
#define int32x8_min _mm256_min_epi32
#define int32x8_max _mm256_max_epi32

#define int32x8_MINMAX(a,b) \
do { \
  int32x8 c = int32x8_min(a,b); \
  b = int32x8_max(a,b); \
  a = c; \
} while(0)


#define APPLY_CONTROL_BIT_32x8(a,b,cb) \
do { \
  int32x8   temp8x32 = a ^ b; \
  temp8x32 &= _mm256_sub_epi32(zero_32x8, cb); \
  a^=temp8x32; \
  b^=temp8x32; \
} while(0)



/* This macro applies VPCOMPRESSW with mask 0b0101010101010101 to
   both vec32x8_1 and vec32x8_2, storing the results in the variable
   vec32x8_1 with the first result in the lower xmm half and the 
   second in the upper xmm half. i.e. representing, 

    vec32x8_1 = | --- p7  --- p6  --- p5  --- p4  --- p3  --- p2  --- p1  --- p0  |
    vec32x8_2 = | --- p15 --- p14 --- p13 --- p12 --- p11 --- p10 --- p9  --- p8  |
  
  Then, the macro gives, 

    vec32x8_1 = | p15 p14 p13 p12 p11 p10 p9  p8  p7  p6  p5  p4  p3  p2  p1  p0  |
*/
#define VPCOMPRESSW_and_concat(vec32x8_1, vec32x8_2) \
  vec32x8_2 <<= 16; \
  vec32x8_1 = _mm256_blend_epi16(vec32x8_1, vec32x8_2, 0b10101010); \
  vec32x8_1 = _mm256_shufflelo_epi16(vec32x8_1, 0b11011000); \
  vec32x8_1 = _mm256_shufflehi_epi16(vec32x8_1, 0b11011000); \
  vec32x8_1 = _mm256_permutevar8x32_epi32(vec32x8_1, swizz_for_VPCOMPRESSW_and_concat); \


#endif