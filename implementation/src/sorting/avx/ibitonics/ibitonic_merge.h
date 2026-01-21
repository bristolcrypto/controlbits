#include "../../../utilities/avx_macros.h"




/* =====================    General Purpose   ===================== */
static void ibitonic_merge_forwards(int32_t *x, int32_t m, int32_t w);
static void ibitonic_merge_backwards(int32_t *x, int32_t m, int32_t w);

static void merge_three_stages_forwards(int32_t *x, int32_t m, int32_t w, int32_t j);
static void merge_two_stages_forwards(int32_t *x, int32_t m, int32_t w, int32_t j);
static void merge_one_stage_forwards(int32_t *x, int32_t m, int32_t w, int32_t j);
static void merge_three_stages_backwards(int32_t *x, int32_t m, int32_t w, int32_t j);
static void merge_two_stages_backwards(int32_t *x, int32_t m, int32_t w, int32_t j);
static void merge_one_stage_backwards(int32_t *x, int32_t m, int32_t w, int32_t j);


/* ===================== w >= 3 ===================== */
static void ibitonic_merge_forwards_w_geq_3(int32_t *x, int32_t m, int32_t w);
static void ibitonic_merge_backwards_w_geq_3(int32_t *x, int32_t m, int32_t w);


/* ===================== w <= 2 ===================== */
static void ibitonic_merge_forwards_w_leq_2(int32_t *x, int32_t m, int32_t w);  
static void ibitonic_merge_backwards_w_leq_2(int32_t *x, int32_t m, int32_t w);
void merge_last_five_stages_w0(int32_t * x, int32x8 direction_mask);
void merge_last_four_stages_w1(int32_t * x, int32x8 direction_mask);
void merge_last_three_stages_w2(int32_t * x, int32x8 direction_mask);



static void ibitonic_merge_forwards(int32_t *x, int32_t m, int32_t w){
  if (w < 3) {
    ibitonic_merge_forwards_w_leq_2(x, m, w);
  } else {
    ibitonic_merge_forwards_w_geq_3(x, m, w);
  }
}

static void ibitonic_merge_backwards(int32_t *x, int32_t m, int32_t w){
  if (w < 3) {
    ibitonic_merge_backwards_w_leq_2(x, m, w);
  } else {
    ibitonic_merge_backwards_w_geq_3(x, m, w);
  }
}


/* Apply three blue layers (see images in docs) */
static void merge_three_stages_forwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-3));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);
      int32x8 x2 = int32x8_load(&x[block+window+2*smallest_jump_size]);
      int32x8 x3 = int32x8_load(&x[block+window+3*smallest_jump_size]);
      int32x8 x4 = int32x8_load(&x[block+window+4*smallest_jump_size]);
      int32x8 x5 = int32x8_load(&x[block+window+5*smallest_jump_size]);
      int32x8 x6 = int32x8_load(&x[block+window+6*smallest_jump_size]);
      int32x8 x7 = int32x8_load(&x[block+window+7*smallest_jump_size]);

      /* Merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x0, x4);
      int32x8_MINMAX(x1, x5);
      int32x8_MINMAX(x2, x6);
      int32x8_MINMAX(x3, x7);

      /* Two merges of jumpsize 2^{w+j-2}. */
      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7); 

      /* Four merges of jumpsize 2^{w+j-3}. */
      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3);
      int32x8_MINMAX(x4, x5);
      int32x8_MINMAX(x6, x7);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);
      int32x8_store(&x[block+window+2*smallest_jump_size],x2);
      int32x8_store(&x[block+window+3*smallest_jump_size],x3);
      int32x8_store(&x[block+window+4*smallest_jump_size],x4);
      int32x8_store(&x[block+window+5*smallest_jump_size],x5);
      int32x8_store(&x[block+window+6*smallest_jump_size],x6);
      int32x8_store(&x[block+window+7*smallest_jump_size],x7);
    }
  }
}

/* Apply two blue layers (see images in docs) */
static void merge_two_stages_forwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-2));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);
      int32x8 x2 = int32x8_load(&x[block+window+2*smallest_jump_size]);
      int32x8 x3 = int32x8_load(&x[block+window+3*smallest_jump_size]);

      /* One merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      /* Two merges of jumpsize 2^{w+j-2}. */
      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);
      int32x8_store(&x[block+window+2*smallest_jump_size],x2);
      int32x8_store(&x[block+window+3*smallest_jump_size],x3);
    }
  }
}

/* Apply one blue layer (see images in docs) */
static void merge_one_stage_forwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-1));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);

      /* One merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x0, x1);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);

    }
  }
}

/* Apply three red layers (see images in docs) */
static void merge_three_stages_backwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-3));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);
      int32x8 x2 = int32x8_load(&x[block+window+2*smallest_jump_size]);
      int32x8 x3 = int32x8_load(&x[block+window+3*smallest_jump_size]);
      int32x8 x4 = int32x8_load(&x[block+window+4*smallest_jump_size]);
      int32x8 x5 = int32x8_load(&x[block+window+5*smallest_jump_size]);
      int32x8 x6 = int32x8_load(&x[block+window+6*smallest_jump_size]);
      int32x8 x7 = int32x8_load(&x[block+window+7*smallest_jump_size]);

      /* Merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x4, x0);
      int32x8_MINMAX(x5, x1);
      int32x8_MINMAX(x6, x2);
      int32x8_MINMAX(x7, x3);

      /* Two merges of jumpsize 2^{w+j-2}. */
      int32x8_MINMAX(x2, x0);
      int32x8_MINMAX(x3, x1);
      int32x8_MINMAX(x6, x4);
      int32x8_MINMAX(x7, x5); 

      /* Four merges of jumpsize 2^{w+j-3}. */
      int32x8_MINMAX(x1, x0);
      int32x8_MINMAX(x3, x2);
      int32x8_MINMAX(x5, x4);
      int32x8_MINMAX(x7, x6);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);
      int32x8_store(&x[block+window+2*smallest_jump_size],x2);
      int32x8_store(&x[block+window+3*smallest_jump_size],x3);
      int32x8_store(&x[block+window+4*smallest_jump_size],x4);
      int32x8_store(&x[block+window+5*smallest_jump_size],x5);
      int32x8_store(&x[block+window+6*smallest_jump_size],x6);
      int32x8_store(&x[block+window+7*smallest_jump_size],x7);
    }
  }
}

/* Apply two red layers (see images in docs) */
static void merge_two_stages_backwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-2));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);
      int32x8 x2 = int32x8_load(&x[block+window+2*smallest_jump_size]);
      int32x8 x3 = int32x8_load(&x[block+window+3*smallest_jump_size]);

      /* One merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x2, x0);
      int32x8_MINMAX(x3, x1);
      /* Two merges of jumpsize 2^{w+j-2}. */
      int32x8_MINMAX(x1, x0);
      int32x8_MINMAX(x3, x2);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);
      int32x8_store(&x[block+window+2*smallest_jump_size],x2);
      int32x8_store(&x[block+window+3*smallest_jump_size],x3);
    }
  }
}

/* Apply one red layer (see images in docs) */
static void merge_one_stage_backwards(int32_t *x, int32_t m, int32_t w, int32_t j) {
  int32_t block;
  int32_t window;
  int32_t smallest_jump_size = (1 << (w+j-1));
  
  for ( block = 0; block < 1 << m; block += 1 << (w+j)) {
    /* This loop merges a contiguous chunk of 2^w+j elements. */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[block+window]);
      int32x8 x1 = int32x8_load(&x[block+window+1*smallest_jump_size]);

      /* One merge of jumpsize 2^{w+j-1}. */
      int32x8_MINMAX(x1, x0);

      int32x8_store(&x[block+window],x0);
      int32x8_store(&x[block+window+1*smallest_jump_size],x1);

    }
  }
}






static void ibitonic_merge_forwards_w_geq_3(int32_t *x, int32_t m, int32_t w) {
    int32_t j = m-w;

    if ( j % 3 == 1 ) {
        /* Do a singular first stage */
        merge_one_stage_forwards(x, m, w, j);
        j--;
    } else if ( j % 3 == 2 ) {
        /* Do two stages to start*/
        merge_two_stages_forwards(x, m, w, j);
        j -= 2;
    }

    /* Now j is a multiple of 3. */
    while ( j > 0 ) {
        merge_three_stages_forwards(x, m, w, j);
        j -= 3;
    }
}

static void ibitonic_merge_backwards_w_geq_3(int32_t *x, int32_t m, int32_t w) {
    int32_t j = m-w;

    if ( j % 3 == 1 ) {
        /* Do a singular first stage */
        merge_one_stage_backwards(x, m, w, j);
        j--;
    } else if ( j % 3 == 2 ) {
        /* Do two stages to start*/
        merge_two_stages_backwards(x, m, w, j);
        j -= 2;
    }
    /* Now j is a multiple of 3. */
    while ( j > 0 ) {
        merge_three_stages_backwards(x, m, w, j);
        j -= 3;
    }
}



static void ibitonic_merge_forwards_w_leq_2(int32_t *x, int32_t m, int32_t w) {
    int32_t n = (1 << m);
    w=0;
    int32_t j = m-w;
    int32x8 forward_mask = _mm256_set1_epi32(0);

    if ( (j-6) % 3 == 1 ) {
        /* Do a singular first stage */
        merge_one_stage_forwards(x, m, w, j);
        j--;
    } else if ( (j-6) % 3 == 2 ) {
        /* Do two stages to start*/
        merge_two_stages_forwards(x, m, w, j);
        j -= 2;
    }
    /* Now j is a multiple of 3. We descend untill we hit j==6.   */
    while ( j > 6 ) {
        merge_three_stages_forwards(x, m, w, j);
        j -= 3;
    }

    /* Now j==6, */
    for ( int block_of_64 = 0; block_of_64 < n; block_of_64 += 64) {
      merge_last_five_stages_w0(x+block_of_64, forward_mask);
    }
}


static void ibitonic_merge_backwards_w_leq_2(int32_t *x, int32_t m, int32_t w) {
    int32_t n = (1 << m);
    int32_t j = m;
    int32x8 reverse_mask = _mm256_set1_epi32(-1);

    if ( (j-6) % 3 == 1 ) {
        /* Do a singular first stage */
        merge_one_stage_backwards(x, m, 0, j);
        j--;
    } else if ( (j-6) % 3 == 2 ) {
        /* Do two stages to start*/
        merge_two_stages_backwards(x, m, 0, j);
        j -= 2;
    }

    /* Now j is a multiple of 3. We descend untill we hit j==4. Then,
       use Bernstein's sort  */
    while ( j > 6 ) {
        merge_three_stages_backwards(x, m, 0, j);
        j -= 3;
    }

    /* Now j==6, */
    if ( w == 0 ) {
    for ( int block_of_64 = 0; block_of_64 < n; block_of_64 += 64) {
      merge_last_five_stages_w0(x+block_of_64, reverse_mask);
    }
  } else if ( w == 1 ) {
    for ( int block_of_64 = 0; block_of_64 < n; block_of_64 += 64) {
      merge_last_four_stages_w1(x+block_of_64, reverse_mask);
  }
}
}


/* Does last five stages worth of merging when w==0. */
void merge_last_five_stages_w0(int32_t * x, int32x8 direction_mask) {
  /* Direction Mask: 
    (0, 0, 0, 0, 0, 0, 0, 0) : normal
    (-1,-1,-1,-1,-1,-1,-1,-1): reversed
  */
  
  int32x8 a0 = int32x8_load(&x[0]);   /* A01234567 */
  int32x8 a1 = int32x8_load(&x[8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[56]);  /* H01234567 */
  
  a0 ^= direction_mask;    
  a1 ^= direction_mask;    
  a2 ^= direction_mask;
  a3 ^= direction_mask;
  a4 ^= direction_mask;
  a5 ^= direction_mask;
  a6 ^= direction_mask;
  a7 ^= direction_mask;    
  
  
  /* Comparisons of length 32. */
  int32x8_MINMAX(a0,a4);
  int32x8_MINMAX(a1,a5);
  int32x8_MINMAX(a2,a6);
  int32x8_MINMAX(a3,a7);

  /* Comparisons of length 16. */
  int32x8_MINMAX(a0,a2);
  int32x8_MINMAX(a1,a3);
  int32x8_MINMAX(a4,a6);
  int32x8_MINMAX(a5,a7);

  /* Comparisons of length 8. */
  int32x8_MINMAX(a0,a1);
  int32x8_MINMAX(a2,a3);
  int32x8_MINMAX(a4,a5);
  int32x8_MINMAX(a6,a7);

  /* Now swizzle for comparisons of length 4, 2, 1. */
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

  /* Comparisons of length 4. */
  int32x8_MINMAX(d0,d4);
  int32x8_MINMAX(d1,d5);
  int32x8_MINMAX(d2,d6);
  int32x8_MINMAX(d3,d7);

  /* Comparisons of length 2. */
  int32x8_MINMAX(d0,d2);
  int32x8_MINMAX(d1,d3);
  int32x8_MINMAX(d4,d6);
  int32x8_MINMAX(d5,d7);

  /* Comparisons of length 1. */
  int32x8_MINMAX(d0,d1);
  int32x8_MINMAX(d2,d3);
  int32x8_MINMAX(d4,d5);
  int32x8_MINMAX(d6,d7);

  int32x8 e0 = _mm256_unpacklo_epi32(d0,d1); /* A01C01E01G01 */ 
  int32x8 e1 = _mm256_unpackhi_epi32(d0,d1); /* B01D01F01H01 */
  int32x8 e2 = _mm256_unpacklo_epi32(d2,d3); /* A23C23E23G23 */
  int32x8 e3 = _mm256_unpackhi_epi32(d2,d3); /* B23D23F23H23 */
  int32x8 e4 = _mm256_unpacklo_epi32(d4,d5); /* A45C45E45G45 */
  int32x8 e5 = _mm256_unpackhi_epi32(d4,d5); /* B45D45F45H45 */
  int32x8 e6 = _mm256_unpacklo_epi32(d6,d7); /* A67C67E67G67 */
  int32x8 e7 = _mm256_unpackhi_epi32(d6,d7); /* B67D67F67H67 */

  int32x8 f0 = _mm256_unpacklo_epi64(e0,e2); /* A0123E0123 */
  int32x8 f1 = _mm256_unpacklo_epi64(e1,e3); /* B0123F0123 */
  int32x8 f2 = _mm256_unpackhi_epi64(e0,e2); /* C0123G0123 */
  int32x8 f3 = _mm256_unpackhi_epi64(e1,e3); /* D0123H0123 */
  int32x8 f4 = _mm256_unpacklo_epi64(e4,e6); /* A4567E4567 */
  int32x8 f5 = _mm256_unpacklo_epi64(e5,e7); /* B4567F4567 */
  int32x8 f6 = _mm256_unpackhi_epi64(e4,e6); /* C0123G0123 */
  int32x8 f7 = _mm256_unpackhi_epi64(e5,e7); /* D0123H0123 */

  int32x8 g0 = _mm256_permute2x128_si256(f0,f4,0x20); /* A01234567 */
  int32x8 g1 = _mm256_permute2x128_si256(f2,f6,0x20); /* B01234567 */
  int32x8 g2 = _mm256_permute2x128_si256(f1,f5,0x20); /* C01234567 */
  int32x8 g3 = _mm256_permute2x128_si256(f3,f7,0x20); /* D01234567 */
  int32x8 g4 = _mm256_permute2x128_si256(f0,f4,0x31); /* E01234567 */
  int32x8 g5 = _mm256_permute2x128_si256(f2,f6,0x31); /* F01234567 */
  int32x8 g6 = _mm256_permute2x128_si256(f1,f5,0x31); /* G01234567 */
  int32x8 g7 = _mm256_permute2x128_si256(f3,f7,0x31); /* H01234567 */

  g0 ^= direction_mask;    
  g1 ^= direction_mask;    
  g2 ^= direction_mask;
  g3 ^= direction_mask;
  g4 ^= direction_mask;
  g5 ^= direction_mask;
  g6 ^= direction_mask;
  g7 ^= direction_mask; 


  int32x8_store(&x[0], g0);
  int32x8_store(&x[8], g1);
  int32x8_store(&x[16],g2);
  int32x8_store(&x[24],g3);
  int32x8_store(&x[32],g4);
  int32x8_store(&x[40],g5);
  int32x8_store(&x[48],g6);
  int32x8_store(&x[56],g7);
}

void merge_last_four_stages_w1(int32_t * x, int32x8 direction_mask) {

  int32x8 a0 = int32x8_load(&x[0]);   /* A01234567 */
  int32x8 a1 = int32x8_load(&x[8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[56]);  /* H01234567 */
  
  a0 ^= direction_mask;    
  a1 ^= direction_mask;    
  a2 ^= direction_mask;
  a3 ^= direction_mask;
  a4 ^= direction_mask;
  a5 ^= direction_mask;
  a6 ^= direction_mask;
  a7 ^= direction_mask;    

  /* Comparisons of length 32. */
  int32x8_MINMAX(a0,a4);
  int32x8_MINMAX(a1,a5);
  int32x8_MINMAX(a2,a6);
  int32x8_MINMAX(a3,a7);

  /* Comparisons of length 16. */
  int32x8_MINMAX(a0,a2);
  int32x8_MINMAX(a1,a3);
  int32x8_MINMAX(a4,a6);
  int32x8_MINMAX(a5,a7);

  /* Comparisons of length 8. */
  int32x8_MINMAX(a0,a1);
  int32x8_MINMAX(a2,a3);
  int32x8_MINMAX(a4,a5);
  int32x8_MINMAX(a6,a7);

  /* Swizzle for comparisons of length 4. */
  int32x8 b0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A0123B0123 */
  int32x8 b1 = _mm256_permute2x128_si256(a0,a1,0x31); /* A4567B4567 */
  int32x8 b2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C0123D0123 */
  int32x8 b3 = _mm256_permute2x128_si256(a2,a3,0x31); /* C4567D4567 */
  int32x8 b4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E0123F0123 */
  int32x8 b5 = _mm256_permute2x128_si256(a4,a5,0x31); /* E4567F4567 */
  int32x8 b6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G0123H0123 */
  int32x8 b7 = _mm256_permute2x128_si256(a6,a7,0x31); /* G4567H4567 */

  /* Apply comparisons of length 4.*/
  int32x8_MINMAX(b0,b1);
  int32x8_MINMAX(b2,b3);
  int32x8_MINMAX(b4,b5);
  int32x8_MINMAX(b6,b7);

  /* Swizzle for comparisons of length 2. */
  int32x8 c0 = _mm256_unpacklo_epi64(b0,b1); /* A0145B0145 */
  int32x8 c1 = _mm256_unpackhi_epi64(b0,b1); /* A2367B2367 */
  int32x8 c2 = _mm256_unpacklo_epi64(b2,b3); /* C0145D0145 */
  int32x8 c3 = _mm256_unpackhi_epi64(b2,b3); /* C2367D2367 */
  int32x8 c4 = _mm256_unpacklo_epi64(b4,b5); /* E0145F0145 */
  int32x8 c5 = _mm256_unpackhi_epi64(b4,b5); /* E2367F2367 */
  int32x8 c6 = _mm256_unpacklo_epi64(b6,b7); /* G0145H0145 */
  int32x8 c7 = _mm256_unpackhi_epi64(b6,b7); /* G2367H2367 */

  /* Apply comparisons of length 2.*/
  int32x8_MINMAX(c0,c1);
  int32x8_MINMAX(c2,c3);
  int32x8_MINMAX(c4,c5);
  int32x8_MINMAX(c6,c7);
  
  /* Unswizzle. */
  int32x8 d0 = _mm256_unpacklo_epi64(c0,c1); /* A0123B0123 */
  int32x8 d1 = _mm256_unpackhi_epi64(c0,c1); /* A4567B4567 */
  int32x8 d2 = _mm256_unpacklo_epi64(c2,c3); /* C0123D0123 */
  int32x8 d3 = _mm256_unpackhi_epi64(c2,c3); /* C4567D4567 */
  int32x8 d4 = _mm256_unpacklo_epi64(c4,c5); /* E0123F0123 */
  int32x8 d5 = _mm256_unpackhi_epi64(c4,c5); /* E4567F4567 */
  int32x8 d6 = _mm256_unpacklo_epi64(c6,c7); /* G0123H0123 */
  int32x8 d7 = _mm256_unpackhi_epi64(c6,c7); /* G4567H4567 */

  int32x8 e0 = _mm256_permute2x128_si256(d0,d1,0x20); /* A01234567 */
  int32x8 e1 = _mm256_permute2x128_si256(d0,d1,0x31); /* B01234567 */
  int32x8 e2 = _mm256_permute2x128_si256(d2,d3,0x20); /* C01234567 */
  int32x8 e3 = _mm256_permute2x128_si256(d2,d3,0x31); /* D01234567 */
  int32x8 e4 = _mm256_permute2x128_si256(d4,d5,0x20); /* E01234567 */
  int32x8 e5 = _mm256_permute2x128_si256(d4,d5,0x31); /* F01234567 */
  int32x8 e6 = _mm256_permute2x128_si256(d6,d7,0x20); /* G01234567 */
  int32x8 e7 = _mm256_permute2x128_si256(d6,d7,0x31); /* H01234567 */

  e0 ^= direction_mask;    
  e1 ^= direction_mask;    
  e2 ^= direction_mask;
  e3 ^= direction_mask;
  e4 ^= direction_mask;
  e5 ^= direction_mask;
  e6 ^= direction_mask;
  e7 ^= direction_mask;

  int32x8_store(&x[0], e0);
  int32x8_store(&x[8], e1);
  int32x8_store(&x[16],e2);
  int32x8_store(&x[24],e3);
  int32x8_store(&x[32],e4);
  int32x8_store(&x[40],e5);
  int32x8_store(&x[48],e6);
  int32x8_store(&x[56],e7);
}

void merge_last_three_stages_w2(int32_t * x, int32x8 direction_mask) {

  int32x8 a0 = int32x8_load(&x[0]);   /* A01234567 */
  int32x8 a1 = int32x8_load(&x[8]);   /* B01234567 */
  int32x8 a2 = int32x8_load(&x[16]);  /* C01234567 */
  int32x8 a3 = int32x8_load(&x[24]);  /* D01234567 */
  int32x8 a4 = int32x8_load(&x[32]);  /* E01234567 */
  int32x8 a5 = int32x8_load(&x[40]);  /* F01234567 */
  int32x8 a6 = int32x8_load(&x[48]);  /* G01234567 */
  int32x8 a7 = int32x8_load(&x[56]);  /* H01234567 */
  
  a0 ^= direction_mask;    
  a1 ^= direction_mask;    
  a2 ^= direction_mask;
  a3 ^= direction_mask;
  a4 ^= direction_mask;
  a5 ^= direction_mask;
  a6 ^= direction_mask;
  a7 ^= direction_mask;    
  
  /* Comparisons of length 32. */
  int32x8_MINMAX(a0,a4);
  int32x8_MINMAX(a1,a5);
  int32x8_MINMAX(a2,a6);
  int32x8_MINMAX(a3,a7);

  /* Comparisons of length 16. */
  int32x8_MINMAX(a0,a2);
  int32x8_MINMAX(a1,a3);
  int32x8_MINMAX(a4,a6);
  int32x8_MINMAX(a5,a7);

  /* Comparisons of length 8. */
  int32x8_MINMAX(a0,a1);
  int32x8_MINMAX(a2,a3);
  int32x8_MINMAX(a4,a5);
  int32x8_MINMAX(a6,a7);

  /* Swizzle for comparisons of length 4. */
  int32x8 b0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A0123B0123 */
  int32x8 b1 = _mm256_permute2x128_si256(a0,a1,0x31); /* A4567B4567 */
  int32x8 b2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C0123D0123 */
  int32x8 b3 = _mm256_permute2x128_si256(a2,a3,0x31); /* C4567D4567 */
  int32x8 b4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E0123F0123 */
  int32x8 b5 = _mm256_permute2x128_si256(a4,a5,0x31); /* E4567F4567 */
  int32x8 b6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G0123H0123 */
  int32x8 b7 = _mm256_permute2x128_si256(a6,a7,0x31); /* G4567H4567 */

  /* Apply comparisons of length 4.*/
  int32x8_MINMAX(b0,b1);
  int32x8_MINMAX(b2,b3);
  int32x8_MINMAX(b4,b5);
  int32x8_MINMAX(b6,b7);
  
  /* Unswizzle */
  int32x8 c0 = _mm256_permute2x128_si256(b0,b1,0x20); /* A01234567 */
  int32x8 c1 = _mm256_permute2x128_si256(b0,b1,0x31); /* B01234567 */
  int32x8 c2 = _mm256_permute2x128_si256(b2,b3,0x20); /* C01234567 */
  int32x8 c3 = _mm256_permute2x128_si256(b2,b3,0x31); /* D01234567 */
  int32x8 c4 = _mm256_permute2x128_si256(b4,b5,0x20); /* E01234567 */
  int32x8 c5 = _mm256_permute2x128_si256(b4,b5,0x31); /* F01234567 */
  int32x8 c6 = _mm256_permute2x128_si256(b6,b7,0x20); /* G01234567 */
  int32x8 c7 = _mm256_permute2x128_si256(b6,b7,0x31); /* H01234567 */
  
  c0 ^= direction_mask;    
  c1 ^= direction_mask;    
  c2 ^= direction_mask;
  c3 ^= direction_mask;
  c4 ^= direction_mask;
  c5 ^= direction_mask;
  c6 ^= direction_mask;
  c7 ^= direction_mask;

  int32x8_store(&x[0], c0);
  int32x8_store(&x[8], c1);
  int32x8_store(&x[16],c2);
  int32x8_store(&x[24],c3);
  int32x8_store(&x[32],c4);
  int32x8_store(&x[40],c5);
  int32x8_store(&x[48],c6);
  int32x8_store(&x[56],c7);
}

