#include "../../../utilities/avx_macros.h"
/* NOTATION:

   We use 'case j' to denote that m - w = j. This implies that the larger
   structure of ibitonic(m, w) is similar to that of ibitonic(i,0), i.e.
   the normal bitonic merge on 2^i elements. 
*/
static void case_2_with_w_geq_3(int32_t *x, int32_t w);
static void case_3_with_w_geq_3(int32_t *x, int32_t w);
static void case_4_with_w_geq_3(int32_t *x, int32_t w);
static void reversed_case_4_with_w_geq_3(int32_t *x, int32_t w);

void chunk_of_64_w0(int32_t * x);
void chunk_of_64_w1(int32_t * x);
void chunk_of_64_w2(int32_t * x);


/*  When m-w == 2, the whole structure looks like that of ibitonic(2, 0).

    This works by sliding 4 windows of size 8 and performing all
    the swaps these windows are involved in sequentially. 
*/
static void case_2_with_w_geq_3(int32_t *x, int32_t w) {
    int32_t window;
    int32_t smallest_jump_size = (1 << w);
    
    for (window = 0; window < smallest_jump_size; window+=8) {
        
        int32x8 x0 = int32x8_load(&x[window]);
        int32x8 x1 = int32x8_load(&x[window+1*smallest_jump_size]);
        int32x8 x2 = int32x8_load(&x[window+2*smallest_jump_size]);
        int32x8 x3 = int32x8_load(&x[window+3*smallest_jump_size]);

        
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);


        int32x8_store(&x[window],x0);
        int32x8_store(&x[window+1*smallest_jump_size],x1);
        int32x8_store(&x[window+2*smallest_jump_size],x2);
        int32x8_store(&x[window+3*smallest_jump_size],x3);
    }
}



/*  When m-w == 3, the whole structure looks like that of ibitonic(3, 0).

    This works by sliding 8 windows of size 8 and performing all
    the swaps these windows are involved in sequentially.
*/
static void case_3_with_w_geq_3(int32_t *x, int32_t w) {
    int32_t window;
    int32_t smallest_jump_size = (1 << w);
    
    for (window = 0; window < smallest_jump_size; window+=8) {
        
        int32x8 x0 = int32x8_load(&x[window]);
        int32x8 x1 = int32x8_load(&x[window+1*smallest_jump_size]);
        int32x8 x2 = int32x8_load(&x[window+2*smallest_jump_size]);
        int32x8 x3 = int32x8_load(&x[window+3*smallest_jump_size]);
        int32x8 x4 = int32x8_load(&x[window+4*smallest_jump_size]);
        int32x8 x5 = int32x8_load(&x[window+5*smallest_jump_size]);
        int32x8 x6 = int32x8_load(&x[window+6*smallest_jump_size]);
        int32x8 x7 = int32x8_load(&x[window+7*smallest_jump_size]);

        /* Apply first stage of sorting. */
        int32x8_MINMAX(x0, x1); 
        int32x8_MINMAX(x3, x2); 
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x7, x6);

        /* Apply second stage of sorting. */
        int32x8_MINMAX(x0, x2); 
        int32x8_MINMAX(x1, x3); 
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3); 
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x7, x6);

        /* Apply merges. */
        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5); 
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3); 
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);

        int32x8_MINMAX(x0, x1); 
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x6, x7);


        int32x8_store(&x[window],x0);
        int32x8_store(&x[window+1*smallest_jump_size],x1);
        int32x8_store(&x[window+2*smallest_jump_size],x2);
        int32x8_store(&x[window+3*smallest_jump_size],x3);
        int32x8_store(&x[window+4*smallest_jump_size],x4);
        int32x8_store(&x[window+5*smallest_jump_size],x5);
        int32x8_store(&x[window+6*smallest_jump_size],x6);
        int32x8_store(&x[window+7*smallest_jump_size],x7);
    }
}



/*  When m-w == 4, the whole structure looks like that of ibitonic(4, 0).
    
    This works by sliding 16 windows of size 8 and performing all
    the swaps these windows are involved in sequentially. This is
    the largest bitonic pattern we can do in the 16 ymm registers
    offered by AVX2.
*/


static void case_4_with_w_geq_3(int32_t *x, int32_t w) {
    int32_t window;
    int32_t smallest_jump_size = (1 << w);
    
    for (window = 0; window < smallest_jump_size; window+=8) {
        
        int32x8 x0 = int32x8_load(&x[window]);
        int32x8 x1 = int32x8_load(&x[window+1*smallest_jump_size]);
        int32x8 x2 = int32x8_load(&x[window+2*smallest_jump_size]);
        int32x8 x3 = int32x8_load(&x[window+3*smallest_jump_size]);
        int32x8 x4 = int32x8_load(&x[window+4*smallest_jump_size]);
        int32x8 x5 = int32x8_load(&x[window+5*smallest_jump_size]);
        int32x8 x6 = int32x8_load(&x[window+6*smallest_jump_size]);
        int32x8 x7 = int32x8_load(&x[window+7*smallest_jump_size]);
        int32x8 x8 = int32x8_load(&x[window+8*smallest_jump_size]);
        int32x8 x9 = int32x8_load(&x[window+9*smallest_jump_size]);
        int32x8 x10 = int32x8_load(&x[window+10*smallest_jump_size]);
        int32x8 x11 = int32x8_load(&x[window+11*smallest_jump_size]);
        int32x8 x12 = int32x8_load(&x[window+12*smallest_jump_size]);
        int32x8 x13 = int32x8_load(&x[window+13*smallest_jump_size]);
        int32x8 x14 = int32x8_load(&x[window+14*smallest_jump_size]);
        int32x8 x15 = int32x8_load(&x[window+15*smallest_jump_size]);


        /* This is machine generated. It is bitonic sort on 16 elements. */
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x7, x6);
        int32x8_MINMAX(x8, x9);
        int32x8_MINMAX(x11, x10);
        int32x8_MINMAX(x12, x13);
        int32x8_MINMAX(x15, x14);
        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x7, x6);
        int32x8_MINMAX(x8, x10);
        int32x8_MINMAX(x9, x11);
        int32x8_MINMAX(x8, x9);
        int32x8_MINMAX(x10, x11);
        int32x8_MINMAX(x14, x12);
        int32x8_MINMAX(x15, x13);
        int32x8_MINMAX(x13, x12);
        int32x8_MINMAX(x15, x14);
        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);
        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x6, x7);
        int32x8_MINMAX(x12, x8);
        int32x8_MINMAX(x13, x9);
        int32x8_MINMAX(x14, x10);
        int32x8_MINMAX(x15, x11);
        int32x8_MINMAX(x10, x8);
        int32x8_MINMAX(x11, x9);
        int32x8_MINMAX(x14, x12);
        int32x8_MINMAX(x15, x13);
        int32x8_MINMAX(x9, x8);
        int32x8_MINMAX(x11, x10);
        int32x8_MINMAX(x13, x12);
        int32x8_MINMAX(x15, x14);
        int32x8_MINMAX(x0, x8);
        int32x8_MINMAX(x1, x9);
        int32x8_MINMAX(x2, x10);
        int32x8_MINMAX(x3, x11);
        int32x8_MINMAX(x4, x12);
        int32x8_MINMAX(x5, x13);
        int32x8_MINMAX(x6, x14);
        int32x8_MINMAX(x7, x15);
        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);
        int32x8_MINMAX(x8, x12);
        int32x8_MINMAX(x9, x13);
        int32x8_MINMAX(x10, x14);
        int32x8_MINMAX(x11, x15);
        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);
        int32x8_MINMAX(x8, x10);
        int32x8_MINMAX(x9, x11);
        int32x8_MINMAX(x12, x14);
        int32x8_MINMAX(x13, x15);
        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x6, x7);
        int32x8_MINMAX(x8, x9);
        int32x8_MINMAX(x10, x11);
        int32x8_MINMAX(x12, x13);
        int32x8_MINMAX(x14, x15);

        int32x8_store(&x[window                      ], x0);
        int32x8_store(&x[window+ 1*smallest_jump_size], x1);
        int32x8_store(&x[window+ 2*smallest_jump_size], x2);
        int32x8_store(&x[window+ 3*smallest_jump_size], x3);
        int32x8_store(&x[window+ 4*smallest_jump_size], x4);
        int32x8_store(&x[window+ 5*smallest_jump_size], x5);
        int32x8_store(&x[window+ 6*smallest_jump_size], x6);
        int32x8_store(&x[window+ 7*smallest_jump_size], x7);
        int32x8_store(&x[window+ 8*smallest_jump_size], x8);
        int32x8_store(&x[window+ 9*smallest_jump_size], x9);
        int32x8_store(&x[window+10*smallest_jump_size],x10);
        int32x8_store(&x[window+11*smallest_jump_size],x11);
        int32x8_store(&x[window+12*smallest_jump_size],x12);
        int32x8_store(&x[window+13*smallest_jump_size],x13);
        int32x8_store(&x[window+14*smallest_jump_size],x14);
        int32x8_store(&x[window+15*smallest_jump_size],x15);
    }
}


static void reversed_case_4_with_w_geq_3(int32_t *x, int32_t w) {
    int32_t window;
    int32_t smallest_jump_size = (1 << w);
    
    for (window = 0; window < smallest_jump_size; window+=8) {
        
        int32x8 x0 = int32x8_load(&x[window]);
        int32x8 x1 = int32x8_load(&x[window+1*smallest_jump_size]);
        int32x8 x2 = int32x8_load(&x[window+2*smallest_jump_size]);
        int32x8 x3 = int32x8_load(&x[window+3*smallest_jump_size]);
        int32x8 x4 = int32x8_load(&x[window+4*smallest_jump_size]);
        int32x8 x5 = int32x8_load(&x[window+5*smallest_jump_size]);
        int32x8 x6 = int32x8_load(&x[window+6*smallest_jump_size]);
        int32x8 x7 = int32x8_load(&x[window+7*smallest_jump_size]);
        int32x8 x8 = int32x8_load(&x[window+8*smallest_jump_size]);
        int32x8 x9 = int32x8_load(&x[window+9*smallest_jump_size]);
        int32x8 x10 = int32x8_load(&x[window+10*smallest_jump_size]);
        int32x8 x11 = int32x8_load(&x[window+11*smallest_jump_size]);
        int32x8 x12 = int32x8_load(&x[window+12*smallest_jump_size]);
        int32x8 x13 = int32x8_load(&x[window+13*smallest_jump_size]);
        int32x8 x14 = int32x8_load(&x[window+14*smallest_jump_size]);
        int32x8 x15 = int32x8_load(&x[window+15*smallest_jump_size]);


        /* This is machine generated. It is reversed bitonic sort on 16 elements. */
        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x6, x7);
        int32x8_MINMAX(x9, x8);
        int32x8_MINMAX(x10, x11);
        int32x8_MINMAX(x13, x12);
        int32x8_MINMAX(x14, x15);
        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x6, x7);
        int32x8_MINMAX(x10, x8);
        int32x8_MINMAX(x11, x9);
        int32x8_MINMAX(x9, x8);
        int32x8_MINMAX(x11, x10);
        int32x8_MINMAX(x12, x14);
        int32x8_MINMAX(x13, x15);
        int32x8_MINMAX(x12, x13);
        int32x8_MINMAX(x14, x15);
        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);
        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);
        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x7, x6);
        int32x8_MINMAX(x8, x12);
        int32x8_MINMAX(x9, x13);
        int32x8_MINMAX(x10, x14);
        int32x8_MINMAX(x11, x15);
        int32x8_MINMAX(x8, x10);
        int32x8_MINMAX(x9, x11);
        int32x8_MINMAX(x12, x14);
        int32x8_MINMAX(x13, x15);
        int32x8_MINMAX(x8, x9);
        int32x8_MINMAX(x10, x11);
        int32x8_MINMAX(x12, x13);
        int32x8_MINMAX(x14, x15);
        int32x8_MINMAX(x8, x0);
        int32x8_MINMAX(x9, x1);
        int32x8_MINMAX(x10, x2);
        int32x8_MINMAX(x11, x3);
        int32x8_MINMAX(x12, x4);
        int32x8_MINMAX(x13, x5);
        int32x8_MINMAX(x14, x6);
        int32x8_MINMAX(x15, x7);
        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);
        int32x8_MINMAX(x12, x8);
        int32x8_MINMAX(x13, x9);
        int32x8_MINMAX(x14, x10);
        int32x8_MINMAX(x15, x11);
        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);
        int32x8_MINMAX(x10, x8);
        int32x8_MINMAX(x11, x9);
        int32x8_MINMAX(x14, x12);
        int32x8_MINMAX(x15, x13);
        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x7, x6);
        int32x8_MINMAX(x9, x8);
        int32x8_MINMAX(x11, x10);
        int32x8_MINMAX(x13, x12);
        int32x8_MINMAX(x15, x14);


        int32x8_store(&x[window                      ], x0);
        int32x8_store(&x[window+ 1*smallest_jump_size], x1);
        int32x8_store(&x[window+ 2*smallest_jump_size], x2);
        int32x8_store(&x[window+ 3*smallest_jump_size], x3);
        int32x8_store(&x[window+ 4*smallest_jump_size], x4);
        int32x8_store(&x[window+ 5*smallest_jump_size], x5);
        int32x8_store(&x[window+ 6*smallest_jump_size], x6);
        int32x8_store(&x[window+ 7*smallest_jump_size], x7);
        int32x8_store(&x[window+ 8*smallest_jump_size], x8);
        int32x8_store(&x[window+ 9*smallest_jump_size], x9);
        int32x8_store(&x[window+10*smallest_jump_size],x10);
        int32x8_store(&x[window+11*smallest_jump_size],x11);
        int32x8_store(&x[window+12*smallest_jump_size],x12);
        int32x8_store(&x[window+13*smallest_jump_size],x13);
        int32x8_store(&x[window+14*smallest_jump_size],x14);
        int32x8_store(&x[window+15*smallest_jump_size],x15);
    }
}



/* 
    This function sorts its input into a bitonic sequence of length 64
    with 32 elements increasing followed by 32 elements decreasing. 
*/
void chunk_of_64_w0(int32_t * x) {
  int32x8 mask1 = _mm256_set1_epi32(-1);
  int32x8 mask2 = _mm256_set_epi32(0, -1, -1, 0, 0, -1, -1, 0);


  int32_t i = 0;
  int32x8 a0 = int32x8_load(&x[i]);     /* A01234567 */
  int32x8 a1 = int32x8_load(&x[i+8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[i+16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[i+24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[i+32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[i+40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[i+48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[i+56]);  /* H01234567 */
  
  /* Masks for obtaining bitonic sequences of size 16. */
  a1 ^= mask1;
  a3 ^= mask1;
  a5 ^= mask1;
  a7 ^= mask1;

  int32x8 b0 = _mm256_unpacklo_epi32(a0,a1); /* AB0AB1AB4AB5 */
  int32x8 b1 = _mm256_unpackhi_epi32(a0,a1); /* AB2AB3AB6AB7 */
  int32x8 b2 = _mm256_unpacklo_epi32(a2,a3); /* CD0CD1CD4CD5 */
  int32x8 b3 = _mm256_unpackhi_epi32(a2,a3); /* CD2CD3CD6CD7 */
  int32x8 b4 = _mm256_unpacklo_epi32(a4,a5); /* EF0EF1EF4EF5 */
  int32x8 b5 = _mm256_unpackhi_epi32(a4,a5); /* EF2EF3EF6EF7 */
  int32x8 b6 = _mm256_unpacklo_epi32(a6,a7); /* GH0GH1GH4GH5 */
  int32x8 b7 = _mm256_unpackhi_epi32(a6,a7); /* GH2GH3GH6GH7 */

  int32x8 c0 = _mm256_unpacklo_epi64(b0,b2); /* ABCD0ABCD4 */
  int32x8 c1 = _mm256_unpacklo_epi64(b1,b3); /* ABCD2ABCD6 */
  int32x8 c2 = _mm256_unpackhi_epi64(b0,b2); /* ABCD1ABCD5 */
  int32x8 c3 = _mm256_unpackhi_epi64(b1,b3); /* ABCD3ABCD7 */
  int32x8 c4 = _mm256_unpacklo_epi64(b4,b6); /* EFGH0EFGH4 */
  int32x8 c5 = _mm256_unpacklo_epi64(b5,b7); /* EFGH2EFGH6 */
  int32x8 c6 = _mm256_unpackhi_epi64(b4,b6); /* EFGH1EFGH5 */
  int32x8 c7 = _mm256_unpackhi_epi64(b5,b7); /* EFGH3EFGH7 */
  
  int32x8 d0 = _mm256_permute2x128_si256(c0,c4,0x20); /* ABCDEFGH0 */
  int32x8 d1 = _mm256_permute2x128_si256(c2,c6,0x20); /* ABCDEFGH1 */
  int32x8 d2 = _mm256_permute2x128_si256(c1,c5,0x20); /* ABCDEFGH2 */
  int32x8 d3 = _mm256_permute2x128_si256(c3,c7,0x20); /* ABCDEFGH3 */
  int32x8 d4 = _mm256_permute2x128_si256(c0,c4,0x31); /* ABCDEFGH4 */
  int32x8 d5 = _mm256_permute2x128_si256(c2,c6,0x31); /* ABCDEFGH5 */
  int32x8 d6 = _mm256_permute2x128_si256(c1,c5,0x31); /* ABCDEFGH6 */
  int32x8 d7 = _mm256_permute2x128_si256(c3,c7,0x31); /* ABCDEFGH7 */

  /* Odd-Even sorting on 8 elements. Due to the current masking, this
     sorts into 4 bitonic sequences of length 16.*/
  int32x8_MINMAX(d0,d1);
  int32x8_MINMAX(d2,d3);
  int32x8_MINMAX(d4,d5);
  int32x8_MINMAX(d6,d7);

  int32x8_MINMAX(d0,d2);
  int32x8_MINMAX(d1,d3);
  int32x8_MINMAX(d4,d6);
  int32x8_MINMAX(d5,d7);
  int32x8_MINMAX(d1,d2);
  int32x8_MINMAX(d5,d6);

  int32x8_MINMAX(d0,d4);
  int32x8_MINMAX(d1,d5);
  int32x8_MINMAX(d2,d6);
  int32x8_MINMAX(d3,d7);

  int32x8_MINMAX(d2,d4);
  int32x8_MINMAX(d3,d5);

  int32x8_MINMAX(d1,d2);
  int32x8_MINMAX(d3,d4);
  int32x8_MINMAX(d5,d6);
  
  /* Of each vector, elements 1, 3, 5, 7 (corresponding to B, D, F, and H) are masked.
     For the merging stage to follow, we need elements     C, D, G, and H to be masked. 
  */
  
  d0 ^= mask2;
  d1 ^= mask2;
  d2 ^= mask2;
  d3 ^= mask2;
  d4 ^= mask2;
  d5 ^= mask2;
  d6 ^= mask2;
  d7 ^= mask2; 
  

  d0 = _mm256_shuffle_epi32(d0, 0b11011000); /* ACBDEGFH0 */
  d1 = _mm256_shuffle_epi32(d1, 0b11011000); /* ACBDEGFH1 */
  d2 = _mm256_shuffle_epi32(d2, 0b11011000); /* ACBDEGFH2 */
  d3 = _mm256_shuffle_epi32(d3, 0b11011000); /* ACBDEGFH3 */
  d4 = _mm256_shuffle_epi32(d4, 0b11011000); /* ACBDEGFH4 */
  d5 = _mm256_shuffle_epi32(d5, 0b11011000); /* ACBDEGFH5 */
  d6 = _mm256_shuffle_epi32(d6, 0b11011000); /* ACBDEGFH6 */
  d7 = _mm256_shuffle_epi32(d7, 0b11011000); /* ACBDEGFH7 */

  int32x8 e0 = _mm256_unpacklo_epi32(d0,d1); /* A01C01E01G01 */ 
  int32x8 e1 = _mm256_unpackhi_epi32(d0,d1); /* B01D01F01H01 */
  int32x8 e2 = _mm256_unpacklo_epi32(d2,d3); /* A23C23E23G23 */
  int32x8 e3 = _mm256_unpackhi_epi32(d2,d3); /* B23D23F23H23 */
  int32x8 e4 = _mm256_unpacklo_epi32(d4,d5); /* A45C45E45G45 */
  int32x8 e5 = _mm256_unpackhi_epi32(d4,d5); /* B45D45F45H45 */
  int32x8 e6 = _mm256_unpacklo_epi32(d6,d7); /* A56C56E56G56 */
  int32x8 e7 = _mm256_unpackhi_epi32(d6,d7); /* B56D56F56H56 */
  
  /* Apply sorts of size 8 for the merge stage. */
  int32x8_MINMAX(e0, e1);
  int32x8_MINMAX(e2, e3);
  int32x8_MINMAX(e4, e5);
  int32x8_MINMAX(e6, e7);
  
  e0 = _mm256_shuffle_epi32(e0, 0b11011000); /* AC0AC1EG0EG1 */
  e1 = _mm256_shuffle_epi32(e1, 0b11011000); /* BD0BD1FH0FH1 */
  e2 = _mm256_shuffle_epi32(e2, 0b11011000); /* AC2AC3EG2EG3 */
  e3 = _mm256_shuffle_epi32(e3, 0b11011000); /* BD2BD3FH2FH3 */
  e4 = _mm256_shuffle_epi32(e4, 0b11011000); /* AC4AC5EG4EG5 */
  e5 = _mm256_shuffle_epi32(e5, 0b11011000); /* BD4BD5FH4FH5 */
  e6 = _mm256_shuffle_epi32(e6, 0b11011000); /* AC6AC7EG6EG7 */
  e7 = _mm256_shuffle_epi32(e7, 0b11011000); /* BD6BD7FH6FH7 */

  int32x8 f0 = _mm256_unpacklo_epi32(e0,e1); /* ABCDEFGH0 */ 
  int32x8 f1 = _mm256_unpackhi_epi32(e0,e1); /* ABCDEFGH1 */
  int32x8 f2 = _mm256_unpacklo_epi32(e2,e3); /* ABCDEFGH2 */
  int32x8 f3 = _mm256_unpackhi_epi32(e2,e3); /* ABCDEFGH3 */
  int32x8 f4 = _mm256_unpacklo_epi32(e4,e5); /* ABCDEFGH4 */
  int32x8 f5 = _mm256_unpackhi_epi32(e4,e5); /* ABCDEFGH5 */
  int32x8 f6 = _mm256_unpacklo_epi32(e6,e7); /* ABCDEFGH6 */
  int32x8 f7 = _mm256_unpackhi_epi32(e6,e7); /* ABCDEFGH7 */
  
  /* Apply sorts of size 4, 2, 1 for merge stage.*/
  int32x8_MINMAX(f0,f4);
  int32x8_MINMAX(f1,f5);
  int32x8_MINMAX(f2,f6);
  int32x8_MINMAX(f3,f7);
  int32x8_MINMAX(f0,f2);
  int32x8_MINMAX(f1,f3);
  int32x8_MINMAX(f4,f6);
  int32x8_MINMAX(f5,f7);
  int32x8_MINMAX(f0,f1);
  int32x8_MINMAX(f2,f3);
  int32x8_MINMAX(f4,f5);
  int32x8_MINMAX(f6,f7);
  
  int32x8 g0 = _mm256_unpacklo_epi32(f0,f1); /* A01C01E01G01 */ 
  int32x8 g1 = _mm256_unpackhi_epi32(f0,f1); /* B01D01F01H01 */
  int32x8 g2 = _mm256_unpacklo_epi32(f2,f3); /* A23C23E23G23 */
  int32x8 g3 = _mm256_unpackhi_epi32(f2,f3); /* B23D23F23H23 */
  int32x8 g4 = _mm256_unpacklo_epi32(f4,f5); /* A45C45E45G45 */
  int32x8 g5 = _mm256_unpackhi_epi32(f4,f5); /* B45D45F45H45 */
  int32x8 g6 = _mm256_unpacklo_epi32(f6,f7); /* A67C67E67G67 */
  int32x8 g7 = _mm256_unpackhi_epi32(f6,f7); /* B67D67F67H67 */

  int32x8 h0 = _mm256_unpacklo_epi64(g0,g2); /* A0123E0123 */
  int32x8 h1 = _mm256_unpacklo_epi64(g1,g3); /* B0123F0123 */
  int32x8 h2 = _mm256_unpackhi_epi64(g0,g2); /* C0123G0123 */
  int32x8 h3 = _mm256_unpackhi_epi64(g1,g3); /* D0123H0123 */
  int32x8 h4 = _mm256_unpacklo_epi64(g4,g6); /* A4567E4567 */
  int32x8 h5 = _mm256_unpacklo_epi64(g5,g7); /* B4567F4567 */
  int32x8 h6 = _mm256_unpackhi_epi64(g4,g6); /* C0123G0123 */
  int32x8 h7 = _mm256_unpackhi_epi64(g5,g7); /* D0123H0123 */

  int32x8 i0 = _mm256_permute2x128_si256(h0,h4,0x20); /* A01234567 */
  int32x8 i1 = _mm256_permute2x128_si256(h2,h6,0x20); /* B01234567 */
  int32x8 i2 = _mm256_permute2x128_si256(h1,h5,0x20); /* C01234567 */
  int32x8 i3 = _mm256_permute2x128_si256(h3,h7,0x20); /* D01234567 */
  int32x8 i4 = _mm256_permute2x128_si256(h0,h4,0x31); /* E01234567 */
  int32x8 i5 = _mm256_permute2x128_si256(h2,h6,0x31); /* F01234567 */
  int32x8 i6 = _mm256_permute2x128_si256(h1,h5,0x31); /* G01234567 */
  int32x8 i7 = _mm256_permute2x128_si256(h3,h7,0x31); /* H01234567 */

  /* We now have two bitonic sequences of 32. We now want one bitonic sequence
     of length 64 so we merge. Elements in C, D, G, and H are masked. We need
     elements E, F, G, and H to be masked. */

  i2 ^= mask1; /* unmasking */
  i3 ^= mask1; /* unmasking */
  i4 ^= mask1; /*   masking */
  i5 ^= mask1; /*   masking */

  /* Apply comparisons of length 16. */
  int32x8_MINMAX(i0, i2);
  int32x8_MINMAX(i1, i3);
  int32x8_MINMAX(i4, i6);
  int32x8_MINMAX(i5, i7);

  /* Apply comparisons of length 8. */
  int32x8_MINMAX(i0, i1);
  int32x8_MINMAX(i2, i3);
  int32x8_MINMAX(i4, i5);
  int32x8_MINMAX(i6, i7);

  /* Shuffle to apply comparisons of lengths 4, 2, 1. */
  int32x8 j0 = _mm256_unpacklo_epi32(i0,i1); /* AB0AB1AB4AB5 */
  int32x8 j1 = _mm256_unpackhi_epi32(i0,i1); /* AB2AB3AB6AB7 */
  int32x8 j2 = _mm256_unpacklo_epi32(i2,i3); /* CD0CD1CD4CD5 */
  int32x8 j3 = _mm256_unpackhi_epi32(i2,i3); /* CD2CD3CD6CD7 */
  int32x8 j4 = _mm256_unpacklo_epi32(i4,i5); /* EF0EF1EF4EF5 */
  int32x8 j5 = _mm256_unpackhi_epi32(i4,i5); /* EF2EF3EF6EF7 */
  int32x8 j6 = _mm256_unpacklo_epi32(i6,i7); /* GH0GH1GH4GH5 */
  int32x8 j7 = _mm256_unpackhi_epi32(i6,i7); /* GH2GH3GH6GH7 */

  int32x8 k0 = _mm256_unpacklo_epi64(j0,j2); /* ABCD0ABCD4 */
  int32x8 k1 = _mm256_unpacklo_epi64(j1,j3); /* ABCD2ABCD6 */
  int32x8 k2 = _mm256_unpackhi_epi64(j0,j2); /* ABCD1ABCD5 */
  int32x8 k3 = _mm256_unpackhi_epi64(j1,j3); /* ABCD3ABCD7 */
  int32x8 k4 = _mm256_unpacklo_epi64(j4,j6); /* EFGH0EFGH4 */
  int32x8 k5 = _mm256_unpacklo_epi64(j5,j7); /* EFGH2EFGH6 */
  int32x8 k6 = _mm256_unpackhi_epi64(j4,j6); /* EFGH1EFGH5 */
  int32x8 k7 = _mm256_unpackhi_epi64(j5,j7); /* EFGH3EFGH7 */
  
  int32x8 l0 = _mm256_permute2x128_si256(k0,k4,0x20); /* ABCDEFGH0 */
  int32x8 l1 = _mm256_permute2x128_si256(k2,k6,0x20); /* ABCDEFGH1 */
  int32x8 l2 = _mm256_permute2x128_si256(k1,k5,0x20); /* ABCDEFGH2 */
  int32x8 l3 = _mm256_permute2x128_si256(k3,k7,0x20); /* ABCDEFGH3 */
  int32x8 l4 = _mm256_permute2x128_si256(k0,k4,0x31); /* ABCDEFGH4 */
  int32x8 l5 = _mm256_permute2x128_si256(k2,k6,0x31); /* ABCDEFGH5 */
  int32x8 l6 = _mm256_permute2x128_si256(k1,k5,0x31); /* ABCDEFGH6 */
  int32x8 l7 = _mm256_permute2x128_si256(k3,k7,0x31); /* ABCDEFGH7 */

  /* Apply merges of sizes 4, 2, 1. */
  int32x8_MINMAX(l0, l4);
  int32x8_MINMAX(l1, l5);
  int32x8_MINMAX(l2, l6);
  int32x8_MINMAX(l3, l7);

  int32x8_MINMAX(l0, l2);
  int32x8_MINMAX(l1, l3);
  int32x8_MINMAX(l4, l6);
  int32x8_MINMAX(l5, l7);

  int32x8_MINMAX(l0, l1);
  int32x8_MINMAX(l2, l3);
  int32x8_MINMAX(l4, l5);
  int32x8_MINMAX(l6, l7);

  /* Undo all the shuffling. */
  int32x8 m0 = _mm256_unpacklo_epi32(l0,l1); /* A01C01E01G01 */ 
  int32x8 m1 = _mm256_unpackhi_epi32(l0,l1); /* B01D01F01H01 */
  int32x8 m2 = _mm256_unpacklo_epi32(l2,l3); /* A23C23E23G23 */
  int32x8 m3 = _mm256_unpackhi_epi32(l2,l3); /* B23D23F23H23 */
  int32x8 m4 = _mm256_unpacklo_epi32(l4,l5); /* A45C45E45G45 */
  int32x8 m5 = _mm256_unpackhi_epi32(l4,l5); /* B45D45F45H45 */
  int32x8 m6 = _mm256_unpacklo_epi32(l6,l7); /* A67C67E67G67 */
  int32x8 m7 = _mm256_unpackhi_epi32(l6,l7); /* B67D67F67H67 */

  int32x8 n0 = _mm256_unpacklo_epi64(m0,m2); /* A0123E0123 */
  int32x8 n1 = _mm256_unpacklo_epi64(m1,m3); /* B0123F0123 */
  int32x8 n2 = _mm256_unpackhi_epi64(m0,m2); /* C0123G0123 */
  int32x8 n3 = _mm256_unpackhi_epi64(m1,m3); /* D0123H0123 */
  int32x8 n4 = _mm256_unpacklo_epi64(m4,m6); /* A4567E4567 */
  int32x8 n5 = _mm256_unpacklo_epi64(m5,m7); /* B4567F4567 */
  int32x8 n6 = _mm256_unpackhi_epi64(m4,m6); /* C0123G0123 */
  int32x8 n7 = _mm256_unpackhi_epi64(m5,m7); /* D0123H0123 */
  
  int32x8 o0 = _mm256_permute2x128_si256(n0,n4,0x20); /* A01234567 */
  int32x8 o1 = _mm256_permute2x128_si256(n2,n6,0x20); /* B01234567 */
  int32x8 o2 = _mm256_permute2x128_si256(n1,n5,0x20); /* C01234567 */
  int32x8 o3 = _mm256_permute2x128_si256(n3,n7,0x20); /* D01234567 */
  int32x8 o4 = _mm256_permute2x128_si256(n0,n4,0x31); /* E01234567 */
  int32x8 o5 = _mm256_permute2x128_si256(n2,n6,0x31); /* F01234567 */
  int32x8 o6 = _mm256_permute2x128_si256(n1,n5,0x31); /* G01234567 */
  int32x8 o7 = _mm256_permute2x128_si256(n3,n7,0x31); /* H01234567 */


  /* Undo masks */
  o4 ^= mask1; 
  o5 ^= mask1; 
  o6 ^= mask1; 
  o7 ^= mask1; 

  int32x8_store(&x[i],   o0);
  int32x8_store(&x[i+8], o1);
  int32x8_store(&x[i+16],o2);
  int32x8_store(&x[i+24],o3);
  int32x8_store(&x[i+32],o4);
  int32x8_store(&x[i+40],o5);
  int32x8_store(&x[i+48],o6);
  int32x8_store(&x[i+56],o7);
}

/* 
    This function sorts its input into two interlaced bitonic sequences of
    length 32. Each with 16 elements increasing followed by 16 elements decreasing. 
*/
void chunk_of_64_w1(int32_t * x) {
  int32x8 mask1 = _mm256_set1_epi32(-1);
  int32x8 mask2 = _mm256_set_epi32(-1, -1, -1, -1,  0,  0,  0,  0);
  int32x8 mask3 = _mm256_set_epi32( 0,  0, -1, -1, -1, -1,  0,  0);


  int32_t i = 0;
  int32x8 a0 = int32x8_load(&x[i]);     /* A01234567 */
  int32x8 a1 = int32x8_load(&x[i+8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[i+16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[i+24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[i+32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[i+40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[i+48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[i+56]);  /* H01234567 */
  
  /* We use bitonic sort throughout. */
  int32x8 b0 = _mm256_unpacklo_epi64(a0,a1); /* A01B01A45B45 */
  int32x8 b1 = _mm256_unpackhi_epi64(a0,a1); /* A23B23A67B67 */
  int32x8 b2 = _mm256_unpacklo_epi64(a2,a3); /* C01D01C45D45 */
  int32x8 b3 = _mm256_unpackhi_epi64(a2,a3); /* C23D23C67D67 */
  int32x8 b4 = _mm256_unpacklo_epi64(a4,a5); /* E01F01E45F45 */
  int32x8 b5 = _mm256_unpackhi_epi64(a4,a5); /* E23F23E67F67 */
  int32x8 b6 = _mm256_unpacklo_epi64(a6,a7); /* G01H01G45H45 */
  int32x8 b7 = _mm256_unpackhi_epi64(a6,a7); /* G23H23G67H67 */

  /* Comparisons of size 2. */
  b0 ^= mask2;  
  b1 ^= mask2;
  b2 ^= mask2;
  b3 ^= mask2;
  b4 ^= mask2;
  b5 ^= mask2;
  b6 ^= mask2;
  b7 ^= mask2;

  int32x8_MINMAX(b0,b1);
  int32x8_MINMAX(b2,b3);
  int32x8_MINMAX(b4,b5);
  int32x8_MINMAX(b6,b7);

  /* We now need all elements of B, D, F, H masked*/
  b0 ^= mask3;  
  b1 ^= mask3;
  b2 ^= mask3;
  b3 ^= mask3;
  b4 ^= mask3;
  b5 ^= mask3;
  b6 ^= mask3;
  b7 ^= mask3;
  
  int32x8 c0 = _mm256_permute2x128_si256(b0,b1,0x20); /* A01B01A23B23 */
  int32x8 c1 = _mm256_permute2x128_si256(b0,b1,0x31); /* A45B45A67B67 */
  int32x8 c2 = _mm256_permute2x128_si256(b2,b3,0x20); /* C01D01C23D23 */
  int32x8 c3 = _mm256_permute2x128_si256(b2,b3,0x31); /* C45D45C67D67 */
  int32x8 c4 = _mm256_permute2x128_si256(b4,b5,0x20); /* E01F01E23F23 */
  int32x8 c5 = _mm256_permute2x128_si256(b4,b5,0x31); /* E45F45E67F67 */
  int32x8 c6 = _mm256_permute2x128_si256(b6,b7,0x20); /* G01H01G23H23 */
  int32x8 c7 = _mm256_permute2x128_si256(b6,b7,0x31); /* G45H45G67H67 */
  
  /* Comparisons of size 4. */
  int32x8_MINMAX(c0, c1);
  int32x8_MINMAX(c2, c3);
  int32x8_MINMAX(c4, c5);
  int32x8_MINMAX(c6, c7);

  int32x8 d0 = _mm256_permute2x128_si256(c0,c1,0x20); /* A01B01A45B45 */
  int32x8 d1 = _mm256_permute2x128_si256(c0,c1,0x31); /* A23B23A67B67 */
  int32x8 d2 = _mm256_permute2x128_si256(c2,c3,0x20); /* C01D01C45D45 */
  int32x8 d3 = _mm256_permute2x128_si256(c2,c3,0x31); /* C23D23C67D67 */
  int32x8 d4 = _mm256_permute2x128_si256(c4,c5,0x20); /* E01F01E45F45 */
  int32x8 d5 = _mm256_permute2x128_si256(c4,c5,0x31); /* E23F23E67F67 */
  int32x8 d6 = _mm256_permute2x128_si256(c6,c7,0x20); /* G01H01G45H45 */
  int32x8 d7 = _mm256_permute2x128_si256(c6,c7,0x31); /* G23H23G67H67 */
  
  /* Comparisons of size 2. */
  int32x8_MINMAX(d0,d1);
  int32x8_MINMAX(d2,d3);
  int32x8_MINMAX(d4,d5);
  int32x8_MINMAX(d6,d7);
  

  int32x8 e0 = _mm256_unpacklo_epi64(d0,d1); /* A01234567 */
  int32x8 e1 = _mm256_unpackhi_epi64(d0,d1); /* B01234567 */
  int32x8 e2 = _mm256_unpacklo_epi64(d2,d3); /* C01234567 */
  int32x8 e3 = _mm256_unpackhi_epi64(d2,d3); /* D01234567 */
  int32x8 e4 = _mm256_unpacklo_epi64(d4,d5); /* E01234567 */
  int32x8 e5 = _mm256_unpackhi_epi64(d4,d5); /* F01234567 */
  int32x8 e6 = _mm256_unpacklo_epi64(d6,d7); /* G01234567 */
  int32x8 e7 = _mm256_unpackhi_epi64(d6,d7); /* H01234567 */
  
  /* We now bitonic merge to obtain four bitonic sequences of length 16. We
     need elements in C, D, G, and H to be masked. We currently have all our
     elements in      B, D, F, and H masked. */

  e1 ^= mask1; /* unmask */
  e2 ^= mask1; /*   mask */
  e5 ^= mask1; /*   mask */
  e6 ^= mask1; /* unmask */

  /* Comparisons of size 8. */
  int32x8_MINMAX(e0,e1);
  int32x8_MINMAX(e2,e3);
  int32x8_MINMAX(e4,e5);
  int32x8_MINMAX(e6,e7);

  int32x8 f0 = _mm256_permute2x128_si256(e0,e1,0x20); /* A0123B0123 */
  int32x8 f1 = _mm256_permute2x128_si256(e0,e1,0x31); /* A4567B4567 */
  int32x8 f2 = _mm256_permute2x128_si256(e2,e3,0x20); /* C0123D0123 */
  int32x8 f3 = _mm256_permute2x128_si256(e2,e3,0x31); /* C4567D4567 */
  int32x8 f4 = _mm256_permute2x128_si256(e4,e5,0x20); /* E0123F0123 */
  int32x8 f5 = _mm256_permute2x128_si256(e4,e5,0x31); /* E4567F4567 */
  int32x8 f6 = _mm256_permute2x128_si256(e6,e7,0x20); /* G0123H0123 */
  int32x8 f7 = _mm256_permute2x128_si256(e6,e7,0x31); /* G4567H4567 */
  
  /* Comparisons of size 4. */
  int32x8_MINMAX(f0,f1);
  int32x8_MINMAX(f2,f3);
  int32x8_MINMAX(f4,f5);
  int32x8_MINMAX(f6,f7);

  int32x8 g0 = _mm256_unpacklo_epi64(f0,f1); /* A0145B0145 */
  int32x8 g1 = _mm256_unpackhi_epi64(f0,f1); /* A2367B2367 */
  int32x8 g2 = _mm256_unpacklo_epi64(f2,f3); /* C0145D0145 */
  int32x8 g3 = _mm256_unpackhi_epi64(f2,f3); /* C2367D2367 */
  int32x8 g4 = _mm256_unpacklo_epi64(f4,f5); /* E0145F0145 */
  int32x8 g5 = _mm256_unpackhi_epi64(f4,f5); /* E2367F2367 */
  int32x8 g6 = _mm256_unpacklo_epi64(f6,f7); /* G0145H0145 */
  int32x8 g7 = _mm256_unpackhi_epi64(f6,f7); /* G2367H2367 */

  /* Comparisons of size 2. */
  int32x8_MINMAX(g0,g1);
  int32x8_MINMAX(g2,g3);
  int32x8_MINMAX(g4,g5);
  int32x8_MINMAX(g6,g7);

  /* unswizzle */
  int32x8 h0 = _mm256_unpacklo_epi64(g0,g1); /* A0123B0123 */
  int32x8 h1 = _mm256_unpackhi_epi64(g0,g1); /* A4567B4567 */
  int32x8 h2 = _mm256_unpacklo_epi64(g2,g3); /* C0123D0123 */
  int32x8 h3 = _mm256_unpackhi_epi64(g2,g3); /* C4567D4567 */
  int32x8 h4 = _mm256_unpacklo_epi64(g4,g5); /* E0123F0123 */
  int32x8 h5 = _mm256_unpackhi_epi64(g4,g5); /* E4567F4567 */
  int32x8 h6 = _mm256_unpacklo_epi64(g6,g7); /* G0123H0123 */
  int32x8 h7 = _mm256_unpackhi_epi64(g6,g7); /* G4567H4567 */


  int32x8 i0 = _mm256_permute2x128_si256(h0,h1,0x20); /* A01234567 */
  int32x8 i1 = _mm256_permute2x128_si256(h0,h1,0x31); /* B01234567 */
  int32x8 i2 = _mm256_permute2x128_si256(h2,h3,0x20); /* C01234567 */
  int32x8 i3 = _mm256_permute2x128_si256(h2,h3,0x31); /* D01234567 */
  int32x8 i4 = _mm256_permute2x128_si256(h4,h5,0x20); /* E01234567 */
  int32x8 i5 = _mm256_permute2x128_si256(h4,h5,0x31); /* F01234567 */
  int32x8 i6 = _mm256_permute2x128_si256(h6,h7,0x20); /* G01234567 */
  int32x8 i7 = _mm256_permute2x128_si256(h6,h7,0x31); /* H01234567 */
  
  /* We now begin our final merge stage of maximum jump size 16. */
  /* unmask */
  i2 ^= mask1; /* unmask */
  i3 ^= mask1; /* unmask */
  i4 ^= mask1; /*   mask */
  i5 ^= mask1; /*   mask*/ 

  /* Merges of length 16, 8. */
  int32x8_MINMAX(i0, i2);
  int32x8_MINMAX(i1, i3);
  int32x8_MINMAX(i4, i6);
  int32x8_MINMAX(i5, i7);

  int32x8_MINMAX(i0, i1);
  int32x8_MINMAX(i2, i3);
  int32x8_MINMAX(i4, i5);
  int32x8_MINMAX(i6, i7);

  int32x8 j0 = _mm256_permute2x128_si256(i0,i1,0x20); /* A0123B0123 */
  int32x8 j1 = _mm256_permute2x128_si256(i0,i1,0x31); /* A4567B4567 */
  int32x8 j2 = _mm256_permute2x128_si256(i2,i3,0x20); /* C0123D0123 */
  int32x8 j3 = _mm256_permute2x128_si256(i2,i3,0x31); /* C4567D4567 */
  int32x8 j4 = _mm256_permute2x128_si256(i4,i5,0x20); /* E0123F0123 */
  int32x8 j5 = _mm256_permute2x128_si256(i4,i5,0x31); /* E4567F4567 */
  int32x8 j6 = _mm256_permute2x128_si256(i6,i7,0x20); /* G0123H0123 */
  int32x8 j7 = _mm256_permute2x128_si256(i6,i7,0x31); /* G4567H4567 */
  
  /* Comparisons of size 4. */
  int32x8_MINMAX(j0,j1);
  int32x8_MINMAX(j2,j3);
  int32x8_MINMAX(j4,j5);
  int32x8_MINMAX(j6,j7);

  int32x8 k0 = _mm256_unpacklo_epi64(j0,j1); /* A0145B0145 */
  int32x8 k1 = _mm256_unpackhi_epi64(j0,j1); /* A2367B2367 */
  int32x8 k2 = _mm256_unpacklo_epi64(j2,j3); /* C0145D0145 */
  int32x8 k3 = _mm256_unpackhi_epi64(j2,j3); /* C2367D2367 */
  int32x8 k4 = _mm256_unpacklo_epi64(j4,j5); /* E0145F0145 */
  int32x8 k5 = _mm256_unpackhi_epi64(j4,j5); /* E2367F2367 */
  int32x8 k6 = _mm256_unpacklo_epi64(j6,j7); /* G0145H0145 */
  int32x8 k7 = _mm256_unpackhi_epi64(j6,j7); /* G2367H2367 */

  /* Comparisons of size 2. */
  int32x8_MINMAX(k0,k1);
  int32x8_MINMAX(k2,k3);
  int32x8_MINMAX(k4,k5);
  int32x8_MINMAX(k6,k7);

  /* unswizzle */
  int32x8 l0 = _mm256_unpacklo_epi64(k0,k1); /* A0123B0123 */
  int32x8 l1 = _mm256_unpackhi_epi64(k0,k1); /* A4567B4567 */
  int32x8 l2 = _mm256_unpacklo_epi64(k2,k3); /* C0123D0123 */
  int32x8 l3 = _mm256_unpackhi_epi64(k2,k3); /* C4567D4567 */
  int32x8 l4 = _mm256_unpacklo_epi64(k4,k5); /* E0123F0123 */
  int32x8 l5 = _mm256_unpackhi_epi64(k4,k5); /* E4567F4567 */
  int32x8 l6 = _mm256_unpacklo_epi64(k6,k7); /* G0123H0123 */
  int32x8 l7 = _mm256_unpackhi_epi64(k6,k7); /* G4567H4567 */


  int32x8 m0 = _mm256_permute2x128_si256(l0,l1,0x20); /* A01234567 */
  int32x8 m1 = _mm256_permute2x128_si256(l0,l1,0x31); /* B01234567 */
  int32x8 m2 = _mm256_permute2x128_si256(l2,l3,0x20); /* C01234567 */
  int32x8 m3 = _mm256_permute2x128_si256(l2,l3,0x31); /* D01234567 */
  int32x8 m4 = _mm256_permute2x128_si256(l4,l5,0x20); /* E01234567 */
  int32x8 m5 = _mm256_permute2x128_si256(l4,l5,0x31); /* F01234567 */
  int32x8 m6 = _mm256_permute2x128_si256(l6,l7,0x20); /* G01234567 */
  int32x8 m7 = _mm256_permute2x128_si256(l6,l7,0x31); /* H01234567 */

  m4 ^= mask1;
  m5 ^= mask1;
  m6 ^= mask1;
  m7 ^= mask1;  

  int32x8_store(&x[i],   m0);
  int32x8_store(&x[i+8], m1);
  int32x8_store(&x[i+16],m2);
  int32x8_store(&x[i+24],m3);
  int32x8_store(&x[i+32],m4);
  int32x8_store(&x[i+40],m5);
  int32x8_store(&x[i+48],m6);
  int32x8_store(&x[i+56],m7);

}

/* 
This function sorts its input into 4 interlaced bitonic sequences of length 16. 
Each with 8 elements increasing followed by 8 elements decreasing. 
*/
void chunk_of_64_w2(int32_t * x) {
  int32x8 mask1 = _mm256_set1_epi32(-1);
  int32_t i = 0;
  int32x8 a0 = int32x8_load(&x[i]);     /* A01234567 */
  int32x8 a1 = int32x8_load(&x[i+8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[i+16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[i+24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[i+32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[i+40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[i+48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[i+56]);  /* H01234567 */
  

  /* In the first stage we need B, D, F, H masked. */
  a1 ^= mask1;  
  a3 ^= mask1;
  a5 ^= mask1;
  a7 ^= mask1;  
  
  int32x8 b0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A0123B0123 */
  int32x8 b1 = _mm256_permute2x128_si256(a0,a1,0x31); /* A4567B4567 */
  int32x8 b2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C0123D0123 */
  int32x8 b3 = _mm256_permute2x128_si256(a2,a3,0x31); /* C4567D4567 */
  int32x8 b4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E0123F0123 */
  int32x8 b5 = _mm256_permute2x128_si256(a4,a5,0x31); /* E4567F4567 */
  int32x8 b6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G0123H0123 */
  int32x8 b7 = _mm256_permute2x128_si256(a6,a7,0x31); /* G4567H4567 */

  /* Comparisons of size 4. */
  int32x8_MINMAX(b0, b1);
  int32x8_MINMAX(b2, b3);
  int32x8_MINMAX(b4, b5);
  int32x8_MINMAX(b6, b7);  
  
  int32x8 c0 = _mm256_permute2x128_si256(b0,b1,0x20); /* A01234567 */
  int32x8 c1 = _mm256_permute2x128_si256(b0,b1,0x31); /* B01234567 */
  int32x8 c2 = _mm256_permute2x128_si256(b2,b3,0x20); /* C01234567 */
  int32x8 c3 = _mm256_permute2x128_si256(b2,b3,0x31); /* D01234567 */
  int32x8 c4 = _mm256_permute2x128_si256(b4,b5,0x20); /* E01234567 */
  int32x8 c5 = _mm256_permute2x128_si256(b4,b5,0x31); /* F01234567 */
  int32x8 c6 = _mm256_permute2x128_si256(b6,b7,0x20); /* G01234567 */
  int32x8 c7 = _mm256_permute2x128_si256(b6,b7,0x31); /* H01234567 */

  /* Change masks in preparation for comparisons of size 8. */
  /* We have: B, D, F, H masked. */
  /* We need: C, D, G, H masked. */

  c1 ^= mask1; /* unmask */
  c2 ^= mask1; /*   mask */
  c5 ^= mask1; /* unmask */
  c6 ^= mask1; /*   mask */

  /* Comparisons of size 8. */
  int32x8_MINMAX(c0, c1);
  int32x8_MINMAX(c2, c3);
  int32x8_MINMAX(c4, c5);
  int32x8_MINMAX(c6, c7);

  
  int32x8 d0 = _mm256_permute2x128_si256(c0,c1,0x20); /* A0123B0123 */
  int32x8 d1 = _mm256_permute2x128_si256(c0,c1,0x31); /* A4567B4567 */
  int32x8 d2 = _mm256_permute2x128_si256(c2,c3,0x20); /* C0123D0123 */
  int32x8 d3 = _mm256_permute2x128_si256(c2,c3,0x31); /* C4567D4567 */
  int32x8 d4 = _mm256_permute2x128_si256(c4,c5,0x20); /* E0123F0123 */
  int32x8 d5 = _mm256_permute2x128_si256(c4,c5,0x31); /* E4567F4567 */
  int32x8 d6 = _mm256_permute2x128_si256(c6,c7,0x20); /* G0123H0123 */
  int32x8 d7 = _mm256_permute2x128_si256(c6,c7,0x31); /* G4567H4567 */

  /* Comparisons of size 4. */
  int32x8_MINMAX(d0, d1);
  int32x8_MINMAX(d2, d3);
  int32x8_MINMAX(d4, d5);
  int32x8_MINMAX(d6, d7);  
  
  int32x8 e0 = _mm256_permute2x128_si256(d0,d1,0x20); /* A01234567 */
  int32x8 e1 = _mm256_permute2x128_si256(d0,d1,0x31); /* B01234567 */
  int32x8 e2 = _mm256_permute2x128_si256(d2,d3,0x20); /* C01234567 */
  int32x8 e3 = _mm256_permute2x128_si256(d2,d3,0x31); /* D01234567 */
  int32x8 e4 = _mm256_permute2x128_si256(d4,d5,0x20); /* E01234567 */
  int32x8 e5 = _mm256_permute2x128_si256(d4,d5,0x31); /* F01234567 */
  int32x8 e6 = _mm256_permute2x128_si256(d6,d7,0x20); /* G01234567 */
  int32x8 e7 = _mm256_permute2x128_si256(d6,d7,0x31); /* H01234567 */

  /* Change masks in preparation for comparisons of size 16. */
  /* We have: C, D, G, H masked. */
  /* We need: E, F, G, H masked. */
  e2 ^= mask1; /* unmask */
  e3 ^= mask1; /* unmask */
  e4 ^= mask1; /*   mask */  
  e5 ^= mask1; /*   mask */  

  /* comparisons of size 16, 8 */
  int32x8_MINMAX(e0, e2);
  int32x8_MINMAX(e1, e3);
  int32x8_MINMAX(e4, e6);
  int32x8_MINMAX(e5, e7);

  int32x8_MINMAX(e0, e1);
  int32x8_MINMAX(e2, e3);
  int32x8_MINMAX(e4, e5);
  int32x8_MINMAX(e6, e7);

  int32x8 f0 = _mm256_permute2x128_si256(e0,e1,0x20); /* A0123B0123 */
  int32x8 f1 = _mm256_permute2x128_si256(e0,e1,0x31); /* A4567B4567 */
  int32x8 f2 = _mm256_permute2x128_si256(e2,e3,0x20); /* C0123D0123 */
  int32x8 f3 = _mm256_permute2x128_si256(e2,e3,0x31); /* C4567D4567 */
  int32x8 f4 = _mm256_permute2x128_si256(e4,e5,0x20); /* E0123F0123 */
  int32x8 f5 = _mm256_permute2x128_si256(e4,e5,0x31); /* E4567F4567 */
  int32x8 f6 = _mm256_permute2x128_si256(e6,e7,0x20); /* G0123H0123 */
  int32x8 f7 = _mm256_permute2x128_si256(e6,e7,0x31); /* G4567H4567 */

  /* Comparisons of size 4. */
  int32x8_MINMAX(f0, f1);
  int32x8_MINMAX(f2, f3);
  int32x8_MINMAX(f4, f5);
  int32x8_MINMAX(f6, f7);  
  
  int32x8 g0 = _mm256_permute2x128_si256(f0,f1,0x20); /* A01234567 */
  int32x8 g1 = _mm256_permute2x128_si256(f0,f1,0x31); /* B01234567 */
  int32x8 g2 = _mm256_permute2x128_si256(f2,f3,0x20); /* C01234567 */
  int32x8 g3 = _mm256_permute2x128_si256(f2,f3,0x31); /* D01234567 */
  int32x8 g4 = _mm256_permute2x128_si256(f4,f5,0x20); /* E01234567 */
  int32x8 g5 = _mm256_permute2x128_si256(f4,f5,0x31); /* F01234567 */
  int32x8 g6 = _mm256_permute2x128_si256(f6,f7,0x20); /* G01234567 */
  int32x8 g7 = _mm256_permute2x128_si256(f6,f7,0x31); /* H01234567 */
  
  /* unmask */

  g4 ^= mask1;
  g5 ^= mask1;
  g6 ^= mask1;
  g7 ^= mask1;

  int32x8_store(&x[i],   g0);
  int32x8_store(&x[i+8], g1);
  int32x8_store(&x[i+16],g2);
  int32x8_store(&x[i+24],g3);
  int32x8_store(&x[i+32],g4);
  int32x8_store(&x[i+40],g5);
  int32x8_store(&x[i+48],g6);
  int32x8_store(&x[i+56],g7);

}
