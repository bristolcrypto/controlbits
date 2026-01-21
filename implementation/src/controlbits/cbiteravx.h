#ifndef ITERATIVEAVX
#define ITERATIVEAVX

#include "../sorting/sort.h"
#include "../utilities/utilities.h"
#include "../utilities/avx_macros.h"
static void cbiteravx(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp); 
static inline void cyclemin_calc(int16_t* pi, int32_t m, int32_t n, int32_t w, int32_t* temp);



/* The cyclemin calculation can be vectorised independent of the value of w. This inline 
   takes over the control bits logic provided that: 
  
      A = x0 = (id << 16) +  pibar_i
      B = anything

    then it will return:

      A = (id<< 16) + pi^-1 
      B = (0 << 16) + fj << w
    
  where fj is the jth control bit of f.
*/
static inline void cyclemin_calc(int16_t* pi, int32_t m, int32_t n, int32_t w, int32_t* temp) {
  int32_t x, j;
  int32x8 x0, x1, x2, x3, id;
  #define A temp
  #define B (temp+n)

  int32x8 id_32x8 = _mm256_set_epi32(7,6,5,4,3,2,1,0);
  int32x8 id_32x8_incr =  _mm256_set1_epi32(8);

  /* We now set A = (id<<16) + pibar^-1 */
  /*            B = (pibar << 16) + c_1 */
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&A[x]);        /* x0 = (id << 16) +  pibar_i */
    x1 = _mm256_srli_epi32(x0, 16);  /* x1 = ( 0 << 16) +       id */
    x2 = _mm256_slli_epi32(x0, 16);  /* x2 = (pibar_i << 16) +   0 */
    x0 = _mm256_min_epi16(x0, x1);   /* x0 = ( 0 << 16) +      c_1 */
    x3 = _mm256_or_si256(x0,x2);     /* x3 = (pibar_i << 16) + c_1 */
    int32x8_store(&B[x], x3);

    x0 = _mm256_or_si256(x1,x2);     /* x0 = (pibar_i << 16) +  id */             
    int32x8_store(&A[x], x0);
  }
  int32_sort_interlaced(A,m,w); 

  /* Set A = (id<<16) + pibar^2 */
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&A[x]);       /* x0 =    (id<<16) + pibar^-1 */
    x1 = int32x8_load(&B[x]);       /* x1 =    (pibar << 16) + c_1 */
    x0 = _mm256_slli_epi32(x0, 16); /* x0 =    (pibar^-1<<16)  + 0 */
    x1 = _mm256_srli_epi32(x1, 16); /* x1 =    (0 << 16) +   pibar */
    x0 = _mm256_or_si256(x0,x1);    /* x0 = (pibar^-1<<16) + pibar */
    int32x8_store(&A[x], x0);
  }
  int32_sort_interlaced(A,m,w);

  /* Set B = (pibar^2 << 16) + c */
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&A[x]);       /* x0 =    (id<<16) +  pibar^2 */
    x1 = int32x8_load(&B[x]);       /* x1 =    (pibar << 16) + c_1 */
    x0 = _mm256_slli_epi32(x0, 16); /* x0 =    (pibar^2<<16) +   0 */
    x1 = _mm256_blend_epi16(x0,x1, 0b01010101);
    int32x8_store(&B[x], x1);
  }

  /* We always begin this loop with B = (p<<16) + c */
  for (j = 1; j < m-w-2 ;++j) {

    /* Set A = (id << 16) + p^-1 */
    id = id_32x8;
    for (x = 0;x < n; x += 8) {
      x0 = int32x8_load(&B[x]);       /* x0 = (p<<16) + c */
      x1 = _mm256_blend_epi16(x0,id, 0b01010101);
      id += id_32x8_incr;
      int32x8_store(&A[x], x1);       /* A =  (p<<16) + x */
    }
    int32_sort_interlaced(A,m,w); 
    
    /* Set A = (p^-1 << 16) + c */
    for (x = 0;x < n; x += 8) {
      x0 = int32x8_load(&A[x]);       /* x0 = (id << 16) + p^-1 */
      x1 = int32x8_load(&B[x]);       /* x1 = (p<<16)    +    c */
      x0 = _mm256_slli_epi32(x0, 16); /* x0 = (p^-1<<16) +    0 */
      x1 = _mm256_blend_epi16(x0,x1, 0b01010101);
      int32x8_store(&A[x], x1);       /*  A = (p^-1<<16) +    c */
    }


    /* Set B = (id<<16)+p^2 */
    for (x = 0;x < n; x += 8) {
      x0 = int32x8_load(&A[x]);       /* x0 = (p^-1<<16)  + c */
      x1 = int32x8_load(&B[x]);       /* x1 = (p<<16)     + c */
      x1 = _mm256_srli_epi32(x1, 16); /* x1 = (0<<16)     + p */
      x0 = _mm256_blend_epi16(x0,x1, 0b01010101);
      int32x8_store(&B[x], x0);       /*  B = (p^(-1)<<16)+ p */
    }

    int32_sort_interlaced(B,m,w); 

    /* Set B = (p^2<<16)+c */
    for (x = 0;x < n; x += 8) {
      x0 = int32x8_load(&A[x]);       /* x0 = (p^-1<<16) +   c */
      x1 = int32x8_load(&B[x]);       /* x1 = (id<<16)   + p^2 */
      x1 = _mm256_slli_epi32(x1, 16); /* x1 = (p^2<<16)  +   0 */
      x0 = _mm256_blend_epi16(x0,x1, 0b10101010);
      int32x8_store(&B[x], x0);       /*  B = (p^2<<16)  +   c */
    }

    /* Set A = (id<<16) + cp */
    int32_sort_interlaced(A,m,w);
        
    /* Set B = (p^2 << 16) + minimum of c and cp */
    for (x = 0;x < n; x += 8) {
      x0 = int32x8_load(&A[x]);       /* x0 = (id<<16)   +  cp */
      x1 = int32x8_load(&B[x]);       /* x1 = (p^2<<16)  +   c */
      x2 = _mm256_min_epi16(x0, x1);  /* x2 = gibberish  + min */
      x0 = _mm256_blend_epi16(x1,x2, 0b01010101);
      int32x8_store(&B[x], x0);       /*  B = (p^2<<16) + min(c,cp) */
    }
  }
  
  /* This code is the final case of the above for loop, i.e. j = (m-w-2). We seperate
     this case out to fill B purely with cmins instead of B = p^2 << 16 + cmin.       */
  
  /* Set A = (id << 16) + p^-1 */
  id = id_32x8;
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&B[x]);       /* x0 = (p<<16) + c */
    x1 = _mm256_blend_epi16(x0,id, 0b01010101);
    id += id_32x8_incr;
    int32x8_store(&A[x], x1);       /* A =  (p<<16) + x */
  }
  int32_sort_interlaced(A,m,w); 
  
  /* Set A = (p^-1 << 16) + c */
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&A[x]);       /* x0 = (id << 16) + p^-1 */
    x1 = int32x8_load(&B[x]);       /* x1 = (p<<16)    +    c */
    x0 = _mm256_slli_epi32(x0, 16); /* x0 = (p^-1<<16) +    0 */
    x1 = _mm256_blend_epi16(x0,x1, 0b01010101);
    int32x8_store(&A[x], x1);       /*  A = (p^-1<<16) +    c */
  }

  /* Set A = (id<<16) + cp */
  int32_sort_interlaced(A,m,w);
  
  /* Set B = ( 0 << 16 ) + (fj << w) */
  int32x8 jump32x8 = _mm256_set1_epi32(1 << w);
  for (x = 0;x < n; x += 8) {
    x0 = int32x8_load(&A[x]);       /* x0 = (id<<16)   +  cp */
    x1 = int32x8_load(&B[x]);       /* x1 = (p^2<<16)  +   c */
    x2 = _mm256_min_epi16(x0, x1);  /* x2 = gibberish  + min */
    x0 = _mm256_and_si256(x2, jump32x8);
    int32x8_store(&B[x], x0);       /*  B = ( 0 << 16 ) + fj << w */
  }
  

  /* These offsets are weird to avoid a permute4x64 instruction. */
  int16x16 offsets = _mm256_set_epi16(15,14,13,12,7,6,5,4,11,10,9,8,3,2,1,0);
  
  /* Convert id of 32x8 into id of 16x16. */
  int16x16 id_16x16_incr = id_32x8_incr << 16;
  id_16x16_incr |= id_32x8_incr;
  id_16x16_incr <<= 1;


  /* Set A = (id<<16) + pi^-1 */
  for (x = 0;x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    x1 = _mm256_unpacklo_epi16(offsets, x0);
    x2 = _mm256_unpackhi_epi16(offsets, x0);
    offsets += id_16x16_incr;
    int32x8_store(&A[x], x1);
    int32x8_store(&A[x+8], x2);
  }
  int32_sort_interlaced(A,m,w); 
}


/* This is a non-recursive implementation of the looping control bits algorithm.
   Parameters:
    *out: Space for (2m-1)n/2 control bits
    *pi: permutation of {0, 1, ..., n-1}
    *temp: must have space for int32_t[2n]
    6 <= m <= 14
    n = 2^m

   Output:
    Control bits for the permutation pi.
    The nth control bit is defined to be 1&(out[n/8] >> (n&7))
*/
static void cbiteravx(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp) {
  int32_t w, j, k, x, pos, jump;
  int32x8 x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,cbs1,cbs2;

  /* This permutation is used to gather our control bits into a form accessible by movemask_epi8 for cases w=0, w=1. */
  __m256i cb_orderer = _mm256_set_epi64x(0xffff0b03ffff0a02, 0xffff0901ffff0800, 0xffff0b03ffff0a02, 0xffff0901ffff0800);
  int32x8 swizz_for_VPCOMPRESSW_and_concat = _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0);
  
  /* This swizzler swaps adjacent int16s. */
  __m256i swap_adjacents = _mm256_set_epi8(0x0d, 0x0c, 0x0f, 0x0e, 0x09, 0x8, 0x0b, 0x0a, 0x05, 0x04, 0x07, 0x06, 0x01, 0x00, 0x03, 0x02, 0x0d, 0x0c, 0x0f, 0x0e, 0x09, 0x8, 0x0b, 0x0a, 0x05, 0x04, 0x07, 0x06, 0x01, 0x00, 0x03, 0x02);
  
  /* For providing the id permutation in int32x8 form. */
  int32x8 id, jump32x8;
  int16x16 jump16x16; 
  int32x8 id_32x8 = _mm256_set_epi32(7,6,5,4,3,2,1,0);
  int32x8 id_32x8_incr =  _mm256_set1_epi32(8);


  /* ========= First iteration of loop where w = 0 ========= */
  pos = 0;
  w = 0;
  jump = 1;
  jump32x8  = _mm256_set1_epi32(jump);
  jump16x16 = _mm256_set1_epi16(jump);

  /* Set A = ( (pi[x]^1) << 16 ) + pi[x^1] */
  for (x=0; x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    x1 = _mm256_shufflehi_epi16(x0, 0b10110001);    
    x1 = _mm256_shufflelo_epi16(x1, 0b10110001);      /* x1 = pi[x^1] */
    x0 ^= jump16x16;                                  /* x0 = pi[x]^1 */
    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    x1 = _mm256_permute4x64_epi64(x1, 0b11011000);
    x2 = _mm256_unpacklo_epi16(x1, x0);             
    x3 = _mm256_unpackhi_epi16(x1, x0);     
    int32x8_store(&A[x],   x2);
    int32x8_store(&A[x+8], x3);
  }

  /* Set A = (id << 16) + pibar_0 */
  int32_sort_interlaced(A,m,w); 
  
  /* Set A = (id<< 16) + pi^-1 
          B = (0 << 16) + fj << w  */
  cyclemin_calc(pi, m, n, w, temp);


  /* Calculate F and set A = (pi^-1 << 16) + F */
  for (x = 0; x < n; x += 64) { 
    /* Get B = (0 << 16) + fj<<w,            comment representation is at the byte level. */
    x0 = int32x8_load(&B[x   ]);   /* x0 = | 000 000 000 f7  000 000 000 f6  000 000 000 f5  000 000 000 f4  000 000 000 f3  000 000 000 f2  000 000 000 f1  000 000 000 f0  |*/
    x1 = int32x8_load(&B[x+ 8]);   /* x1 = | 000 000 000 f15 000 000 000 f14 000 000 000 f13 000 000 000 f12 000 000 000 f11 000 000 000 f10 000 000 000 f9  000 000 000 f8  |*/
    x2 = int32x8_load(&B[x+16]);   /* x2 = | 000 000 000 f23 000 000 000 f22 000 000 000 f21 000 000 000 f20 000 000 000 f19 000 000 000 f18 000 000 000 f17 000 000 000 f16 |*/
    x3 = int32x8_load(&B[x+24]);   /* x3 = | 000 000 000 f31 000 000 000 f30 000 000 000 f29 000 000 000 f28 000 000 000 f27 000 000 000 f26 000 000 000 f25 000 000 000 f24 |*/
    
    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x   ]);
    x5 = int32x8_load(&A[x+ 8]);
    x6 = int32x8_load(&A[x+16]);
    x7 = int32x8_load(&A[x+24]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);

    /* Set A = (pi^-1 <<16) +  F */
    /* The shuffle copies the control bit 2j into 2j+1's place. */
    x4 ^= _mm256_shuffle_epi32(x0, 0b10100000);
    x5 ^= _mm256_shuffle_epi32(x1, 0b10100000);
    x6 ^= _mm256_shuffle_epi32(x2, 0b10100000);
    x7 ^= _mm256_shuffle_epi32(x3, 0b10100000); 

    int32x8_store(&A[x   ], x4);
    int32x8_store(&A[x+ 8], x5);
    int32x8_store(&A[x+16], x6);
    int32x8_store(&A[x+24], x7);

    /* We accumulate control bits and save them in cb1. */
    /* At #1, we set:
        #1: x0 = | 000 000 f15 f7  000 000 f14 f6  000 000 f13 f5  000 000 f12 f4  000 000 f11 f3  000 000 f10 f2  000 000 f9  f1  000 000 f8  f0  |
        #2: x2 = | f31 f23 000 000 f30 f22 000 000 f29 f21 000 000 f28 f20 000 000 f27 f19 000 000 f26 f18 000 000 f25 f17 000 000 f24 f16 000 000 |
        #3:cbs1= | 000 000 f30 f28 000 000 f22 f20 000 000 f14 f12 000 000 f06 f04 000 000 f26 f24 000 000 f18 f16 000 000 f10 f08 000 000 f2  f0  |
    */
    x1 = _mm256_slli_epi32(x1, 8); 
    x0 ^= x1;  /* #1 */
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16; /* #2 */
    x0 ^= x2;        
    cbs1 = _mm256_shuffle_epi8(x0, cb_orderer); /* #3 */
  
    /* Again we calculate F for the higher values. */
    x0 = int32x8_load(&B[x+32]); 
    x1 = int32x8_load(&B[x+40]);
    x2 = int32x8_load(&B[x+48]);
    x3 = int32x8_load(&B[x+56]);

    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x+32]);
    x5 = int32x8_load(&A[x+40]);
    x6 = int32x8_load(&A[x+48]);
    x7 = int32x8_load(&A[x+56]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);

    /* Set A = (pi^-1 <<16) +  F */
    /* The shuffle copies the control bit 2j into 2j+1's place. */
    x4 ^= _mm256_shuffle_epi32(x0, 0b10100000);
    x5 ^= _mm256_shuffle_epi32(x1, 0b10100000);
    x6 ^= _mm256_shuffle_epi32(x2, 0b10100000);
    x7 ^= _mm256_shuffle_epi32(x3, 0b10100000); 

    int32x8_store(&A[x+32], x4);
    int32x8_store(&A[x+40], x5);
    int32x8_store(&A[x+48], x6);
    int32x8_store(&A[x+56], x7);

    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;
    x3 =  _mm256_slli_epi32(x3, 8);                       
    x2 ^= x3; 
    x2 <<= 16;                               
    x0 ^= x2;                                
    cbs2 = _mm256_shuffle_epi8(x0, cb_orderer);


    /* cbs1 = | 000 000 f30 f28 000 000 f22 f20 000 000 f14 f12 000 000 f06 f04 000 000 f26 f24 000 000 f18 f16 000 000 f10 f08 000 000 f2  f0  | */
    /* cbs2 = | 000 000 f62 f60 000 000 f54 f52 000 000 f46 f44 000 000 f48 f36 000 000 f58 f56 000 000 f50 f48 000 000 f42 f40 000 000 f34 f32 | */
    x0 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    x1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00110001);
    x1 <<= 16;
    cbs1 = x0 ^ x1;
    /* cbs1 = | f62 f60 f58 f56 f54 f52 f50 f48 f46 f44 f42 f40 f38 f36 f34 f32 f30 f28 f26 f24 f22 f20 f18 f16 f14 f12 f10 f08 f06 f04 f02 f00 |*/        
    
    /* Align at bit-level to extract control bits with movemask. */
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 7);
    pos += 1;
  }
    
  /* Set A = (id<<16) + F(pi) */
  int32_sort_interlaced(A,m,w);

  /* Since we deference as in32_t pointer AND aren't counting bits, divide scalar value by 32. */
  pos = (2*m-2) * n >> 6;
  /* Calculate l and set pi = M = FpiL. */
  for (x = 0; x < n; x += 64) {
    /* Get A = (id<<16) + F(pi) */
    x0 = int32x8_load(&A[x   ]);
    x1 = int32x8_load(&A[x+ 8]);
    x2 = int32x8_load(&A[x+16]);
    x3 = int32x8_load(&A[x+24]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;                           /* x4 = | 000 l7  000 l6  000 l5  000 l4  000 l3  000 l2  000 l1  000 l0  |*/
    x5 = x1 & jump32x8;                           /* x5 = | 000 l15 000 l14 000 l13 000 l12 000 l11 000 l10 000 l9  000 l8  |*/
    x6 = x2 & jump32x8;                           /* x6 = | 000 l23 000 l22 000 l21 000 l20 000 l19 000 l18 000 l17 000 l16 |*/
    x7 = x3 & jump32x8;                           /* x7 = | 000 l31 000 l30 000 l29 000 l28 000 l27 000 l26 000 l25 000 l24 |*/
    
    /* Copy the control bit 2j into 2j+1's place. */
    x4 = _mm256_shuffle_epi32(x4, 0b10100000);    /* x4 = | 000 l6  000 l6  000 l4  000 l4  000 l2  000 l2  000 l0  000 l0  |*/
    x5 = _mm256_shuffle_epi32(x5, 0b10100000);    /* x5 = | 000 l14 000 l14 000 l12 000 l12 000 l10 000 l10 000 l8  000 l8  |*/
    x6 = _mm256_shuffle_epi32(x6, 0b10100000);    /* x6 = | 000 l22 000 l22 000 l20 000 l20 000 l18 000 l18 000 l16 000 l16 |*/
    x7 = _mm256_shuffle_epi32(x7, 0b10100000);    /* x7 = | 000 l30 000 l30 000 l28 000 l28 000 l26 000 l26 000 l24 000 l24 |*/

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL.  */
    x0 = _mm256_permutevar8x32_epi32(x0, x4^id_32x8);     /* x4 = | ggg p7  ggg p6  ggg p5  ggg p4  ggg p3  ggg p2  ggg p1  ggg p0  |*/
    x1 = _mm256_permutevar8x32_epi32(x1, x5^id_32x8);     /* x5 = | ggg p15 ggg p14 ggg p13 ggg p12 ggg p11 ggg p10 ggg p9  ggg p8  |*/
    x2 = _mm256_permutevar8x32_epi32(x2, x6^id_32x8);     /* x6 = | ggg p23 ggg p22 ggg p21 ggg p20 ggg p19 ggg p18 ggg p17 ggg p16 |*/
    x3 = _mm256_permutevar8x32_epi32(x3, x7^id_32x8);     /* x7 = | ggg p31 ggg p30 ggg p29 ggg p28 ggg p27 ggg p26 ggg p25 ggg p24 |*/

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);

    int32x8_store(&pi[x   ], x0);
    int32x8_store(&pi[x+16], x2);

    /* We accumulate control bits lj and save them in cb1. */
    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;          /* x4   = | -- -- 15 07 | -- -- 14 06 | -- -- 13 05 | -- -- 12 04 | -- -- 11 03 | -- -- 10 02 | -- -- 09 01 | -- -- 08 00 | */
    x7 = _mm256_slli_epi32(x7, 8);
    x6 ^= x7; 
    x6 <<= 16;         /* x6   = | 31 23 -- -- | 30 22 -- -- | 29 21 -- -- | 28 20 -- -- | 27 19 -- -- | 26 18 -- -- | 25 17 -- -- | 24 16 -- -- | */
    x4 ^= x6;          /* x4   = | 31 23 15 07 | 30 22 14 06 | 29 21 13 05 | 28 20 12 04 | 27 19 11 03 | 26 18 10 02 | 25 17 09 01 | 24 16 08 00 | */
    cbs1 = _mm256_shuffle_epi8(x4, cb_orderer);
    /* cbs1  = | -- -- 30 28 | -- -- 22 20 | -- -- 14 12 | -- -- 06 04 | -- -- 26 24 | -- -- 18 16 | -- -- 10 08 | -- -- 02 00 | */

    x0 = int32x8_load(&A[x+32]);
    x1 = int32x8_load(&A[x+40]);
    x2 = int32x8_load(&A[x+48]);
    x3 = int32x8_load(&A[x+56]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;                           /* x4 = | 000 l7  000 l6  000 l5  000 l4  000 l3  000 l2  000 l1  000 l0  |*/
    x5 = x1 & jump32x8;                           /* x5 = | 000 l15 000 l14 000 l13 000 l12 000 l11 000 l10 000 l9  000 l8  |*/
    x6 = x2 & jump32x8;                           /* x6 = | 000 l23 000 l22 000 l21 000 l20 000 l19 000 l18 000 l17 000 l16 |*/
    x7 = x3 & jump32x8;                           /* x7 = | 000 l31 000 l30 000 l29 000 l28 000 l27 000 l26 000 l25 000 l24 |*/
    
    /* Copy the control bit 2j into 2j+1's place. */
    x4 = _mm256_shuffle_epi32(x4, 0b10100000);    /* x4 = | 000 l6  000 l6  000 l4  000 l4  000 l2  000 l2  000 l0  000 l0  |*/
    x5 = _mm256_shuffle_epi32(x5, 0b10100000);    /* x5 = | 000 l14 000 l14 000 l12 000 l12 000 l10 000 l10 000 l8  000 l8  |*/
    x6 = _mm256_shuffle_epi32(x6, 0b10100000);    /* x6 = | 000 l22 000 l22 000 l20 000 l20 000 l18 000 l18 000 l16 000 l16 |*/
    x7 = _mm256_shuffle_epi32(x7, 0b10100000);    /* x7 = | 000 l30 000 l30 000 l28 000 l28 000 l26 000 l26 000 l24 000 l24 |*/

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL */
    x0 = _mm256_permutevar8x32_epi32(x0, x4 ^ id_32x8);     /* x4 = | ggg p7  ggg p6  ggg p5  ggg p4  ggg p3  ggg p2  ggg p1  ggg p0  |*/
    x1 = _mm256_permutevar8x32_epi32(x1, x5 ^ id_32x8);     /* x5 = | ggg p15 ggg p14 ggg p13 ggg p12 ggg p11 ggg p10 ggg p9  ggg p8  |*/
    x2 = _mm256_permutevar8x32_epi32(x2, x6 ^ id_32x8);     /* x6 = | ggg p23 ggg p22 ggg p21 ggg p20 ggg p19 ggg p18 ggg p17 ggg p16 |*/
    x3 = _mm256_permutevar8x32_epi32(x3, x7 ^ id_32x8);     /* x7 = | ggg p31 ggg p30 ggg p29 ggg p28 ggg p27 ggg p26 ggg p25 ggg p24 |*/

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    int32x8_store(&pi[x+32], x0);
    int32x8_store(&pi[x+48], x2);

    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;
    x7 = _mm256_slli_epi32(x7, 8);                       
    x6 ^= x7; 
    x6 <<= 16;                               
    x4 ^= x6;                                
    cbs2 = _mm256_shuffle_epi8(x4, cb_orderer);


    /* cbs1 = | -- -- 30 28 | -- -- 22 20 | -- -- 14 12 | -- -- 06 04 | -- -- 26 24 | -- -- 18 16 | -- -- 10 08 | -- -- 02 00 | */
    /* cbs2 = | -- -- 62 60 | -- -- 54 52 | -- -- 46 44 | -- -- 38 36 | -- -- 58 56 | -- -- 50 48 | -- -- 42 40 | -- -- 34 32 | */
    x0 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    x1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00110001);
    x1 <<= 16;
    cbs1 = x0 ^ x1; 
    /* cbs1 = | 62 60 58 56 | 54 52 50 48 | 46 44 42 40 | 38 36 34 32 | 30 28 26 24 | 22 20 18 16 | 14 12 10 08 | 06 04 02 00 |*/        
    
    /* Align to extract control bits with movemask. */
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 7);
    pos += 1;
  }

  /* =========   END of first iteration of loop where w = 0  ========= */
  /* ========= Start of second iteration of loop where w = 1 ========= */
  pos = n >> 6;
  w = 1;
  jump = 2;
  jump32x8 <<= 1;
  jump16x16 <<= 1; 
  
  /* Set A = ( (pi[x]^2) << 16 ) + pi[x^2] */
  for (x=0; x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    x1 = _mm256_shufflehi_epi16(x0, 0b01001110);    
    x1 = _mm256_shufflelo_epi16(x1, 0b01001110);      /* x1 = pi[x^2] */
    x0 ^= jump16x16;                                  /* x0 = pi[x]^2 */

    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    x1 = _mm256_permute4x64_epi64(x1, 0b11011000);
    x2 = _mm256_unpacklo_epi16(x1, x0);             
    x3 = _mm256_unpackhi_epi16(x1, x0);     
    int32x8_store(&A[x],   x2);
    int32x8_store(&A[x+8], x3);
  }
  
  int32_sort_interlaced(A,m,w); /* A = (id << 16) + pibar_1 */
  cyclemin_calc(pi, m, n, w, temp);
  
  /* Calculate F and set A = (pi^-1 << 16) + F */
  id = id_32x8;
  for (x = 0; x < n; x += 64) {
    
    /* Load in control bits. */
    x0 = int32x8_load(&B[x   ]);
    x1 = int32x8_load(&B[x+ 8]);
    x2 = int32x8_load(&B[x+16]);
    x3 = int32x8_load(&B[x+24]);

    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x   ]);
    x5 = int32x8_load(&A[x+ 8]);
    x6 = int32x8_load(&A[x+16]);
    x7 = int32x8_load(&A[x+24]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);

    /* Set A = (pi^-1 <<16) +  F */
    /* The shuffle copies the control bits 4j, 4j+1 into 4j+2, 4j+3 resp. */
    x4 ^= _mm256_shuffle_epi32(x0, 0b01000100);
    x5 ^= _mm256_shuffle_epi32(x1, 0b01000100);
    x6 ^= _mm256_shuffle_epi32(x2, 0b01000100);
    x7 ^= _mm256_shuffle_epi32(x3, 0b01000100);
    
    int32x8_store(&A[x   ], x4);
    int32x8_store(&A[x+ 8], x5);
    int32x8_store(&A[x+16], x6);
    int32x8_store(&A[x+24], x7);

    /* Now we store the control bits. */


    /* Shuffle to avoid altering cb_orderer. */
    x0 = _mm256_shuffle_epi32(x0, 0b11011000); /* x0       = | -- -- -- 07 | -- -- -- 05 | -- -- -- 06 | -- -- -- 04 | -- -- -- 03 | -- -- -- 01 | -- -- -- 02 | -- -- -- 00 | */
    x1 = _mm256_shuffle_epi32(x1, 0b11011000); /* x1 <<  8 = | -- -- 15 -- | -- -- 13 -- | -- -- 14 -- | -- -- 12 -- | -- -- 11 -- | -- -- 09 -- | -- -- 10 -- | -- -- 08 -- | */
    x2 = _mm256_shuffle_epi32(x2, 0b11011000); /* x2 << 16 = | -- 23 -- -- | -- 21 -- -- | -- 22 -- -- | -- 20 -- -- | -- 19 -- -- | -- 17 -- -- | -- 18 -- -- | -- 16 -- -- | */
    x3 = _mm256_shuffle_epi32(x3, 0b11011000); /* x3 << 24 = | 31 -- -- -- | 29 -- -- -- | 30 -- -- -- | 28 -- -- -- | 27 -- -- -- | 25 -- -- -- | 26 -- -- -- | 24 -- -- -- | */

    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;
    x0 ^= x2;
    
    /* x0 =  | 31 23 15 07 | 29 21 13 05 | 30 22 14 06 | 28 20 12 04 | 27 19 11 03 | 25 17 09 01 | 26 18 10 02 | 24 16 08 00 | */
    cbs1 = _mm256_shuffle_epi8(x0, cb_orderer);
    /* cbs1= | -- -- 29 28 | -- -- 21 20 | -- -- 13 12 | -- -- 05 04 | -- -- 25 24 | -- -- 17 16 | -- -- 09 08 | -- -- 01 00 | */
    
    /* Load in next set of control bits. */
    x0 = int32x8_load(&B[x+32]);
    x1 = int32x8_load(&B[x+40]);
    x2 = int32x8_load(&B[x+48]);
    x3 = int32x8_load(&B[x+56]);

    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x+32]);
    x5 = int32x8_load(&A[x+40]);
    x6 = int32x8_load(&A[x+48]);
    x7 = int32x8_load(&A[x+56]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);

    /* Set A = (pi^-1 <<16) +  F */
    /* The shuffle copies the control bits 4j, 4j+1 into 4j+2, 4j+3 resp. */
    x4 ^= _mm256_shuffle_epi32(x0, 0b01000100);
    x5 ^= _mm256_shuffle_epi32(x1, 0b01000100);
    x6 ^= _mm256_shuffle_epi32(x2, 0b01000100);
    x7 ^= _mm256_shuffle_epi32(x3, 0b01000100);

    int32x8_store(&A[x+32], x4);
    int32x8_store(&A[x+40], x5);
    int32x8_store(&A[x+48], x6);
    int32x8_store(&A[x+56], x7);
    
    /* Adjustment to avoid changing cb_orderer. */
    x0 = _mm256_shuffle_epi32(x0, 0b11011000);
    x1 = _mm256_shuffle_epi32(x1, 0b11011000);
    x2 = _mm256_shuffle_epi32(x2, 0b11011000);
    x3 = _mm256_shuffle_epi32(x3, 0b11011000);

    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;
    x0 ^= x2;
    cbs2 = _mm256_shuffle_epi8(x0, cb_orderer);

    /* cbs1 = | -- -- 29 28 | -- -- 21 20 | -- -- 13 12 | -- -- 05 04 | -- -- 25 24 | -- -- 17 16 | -- -- 09 08 | -- -- 01 00 | */
    /* cbs2 = | -- -- 61 60 | -- -- 53 52 | -- -- 45 44 | -- -- 37 36 | -- -- 57 56 | -- -- 49 48 | -- -- 41 40 | -- -- 33 32 | */
    
    x0 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    x1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00110001);
    x1 <<= 16;
    cbs1 = x0 ^ x1; 

    /* cbs1 = | 61 60 57 56 | 53 52 49 48 | 45 44 41 40 | 37 36 33 32 | 29 28 25 24 | 21 20 17 16 | 13 12 09 08 | 05 04 01 00 | */

    /* Align at bit-level to extract control bits with movemask. */
    ((int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 6);
    pos += 1;
  }
  
  /* Set A = (id<<16) + F(pi) */
  int32_sort_interlaced(A,m,w);
  
  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-3) * (n >> 6);
  for (x = 0; x < n; x +=64) {
    x0 = int32x8_load(&A[x   ]);
    x1 = int32x8_load(&A[x+ 8]);
    x2 = int32x8_load(&A[x+16]);
    x3 = int32x8_load(&A[x+24]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;                                   /* x4 = | 000 l7  000 l6  000 l5  000 l4  000 l3  000 l2  000 l1  000 l0  |*/
    x5 = x1 & jump32x8;                                   /* x5 = | 000 l15 000 l14 000 l13 000 l12 000 l11 000 l10 000 l9  000 l8  |*/
    x6 = x2 & jump32x8;                                   /* x6 = | 000 l23 000 l22 000 l21 000 l20 000 l19 000 l18 000 l17 000 l16 |*/
    x7 = x3 & jump32x8;                                   /* x7 = | 000 l31 000 l30 000 l29 000 l28 000 l27 000 l26 000 l25 000 l24 |*/

    /* Copy the control bit 4j, 4j+1 into 4j+2, 4j+3 resp. */
    x4 = _mm256_shuffle_epi32(x4, 0b01000100);            /* x4 = | 000 l5  000 l4  000 l5  000 l4  000 l1  000 l0  000 l1  000 l0  |*/
    x5 = _mm256_shuffle_epi32(x5, 0b01000100);            /* x5 = | 000 l13 000 l12 000 l13 000 l12 000 l9  000 l8  000 l9  000 l8  |*/
    x6 = _mm256_shuffle_epi32(x6, 0b01000100);            /* x6 = | 000 l21 000 l20 000 l21 000 l20 000 l17 000 l16 000 l17 000 l16 |*/
    x7 = _mm256_shuffle_epi32(x7, 0b01000100);            /* x7 = | 000 l29 000 l28 000 l29 000 l28 000 l25 000 l24 000 l25 000 l24 |*/

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL.  */
    x0 = _mm256_permutevar8x32_epi32(x0, x4^id_32x8);     /* x4 = | ggg p7  ggg p6  ggg p5  ggg p4  ggg p3  ggg p2  ggg p1  ggg p0  |*/
    x1 = _mm256_permutevar8x32_epi32(x1, x5^id_32x8);     /* x5 = | ggg p15 ggg p14 ggg p13 ggg p12 ggg p11 ggg p10 ggg p9  ggg p8  |*/
    x2 = _mm256_permutevar8x32_epi32(x2, x6^id_32x8);     /* x6 = | ggg p23 ggg p22 ggg p21 ggg p20 ggg p19 ggg p18 ggg p17 ggg p16 |*/
    x3 = _mm256_permutevar8x32_epi32(x3, x7^id_32x8);     /* x7 = | ggg p31 ggg p30 ggg p29 ggg p28 ggg p27 ggg p26 ggg p25 ggg p24 |*/

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    int32x8_store(&pi[x   ], x0);
    int32x8_store(&pi[x+16], x2);

    /* We accumulate control bits lj and save them in cb1. */
    x4 = _mm256_shuffle_epi32(x4, 0b11011000);
    x5 = _mm256_shuffle_epi32(x5, 0b11011000);
    x6 = _mm256_shuffle_epi32(x6, 0b11011000);
    x7 = _mm256_shuffle_epi32(x7, 0b11011000);
    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;          
    x7 = _mm256_slli_epi32(x7, 8);
    x6 ^= x7; 
    x6 <<= 16;         
    x4 ^= x6;          
    cbs1 = _mm256_shuffle_epi8(x4, cb_orderer);

    x0 = int32x8_load(&A[x+32]);
    x1 = int32x8_load(&A[x+40]);
    x2 = int32x8_load(&A[x+48]);
    x3 = int32x8_load(&A[x+56]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;                           /* x4 = | 000 l7  000 l6  000 l5  000 l4  000 l3  000 l2  000 l1  000 l0  |*/
    x5 = x1 & jump32x8;                           /* x5 = | 000 l15 000 l14 000 l13 000 l12 000 l11 000 l10 000 l9  000 l8  |*/
    x6 = x2 & jump32x8;                           /* x6 = | 000 l23 000 l22 000 l21 000 l20 000 l19 000 l18 000 l17 000 l16 |*/
    x7 = x3 & jump32x8;                           /* x7 = | 000 l31 000 l30 000 l29 000 l28 000 l27 000 l26 000 l25 000 l24 |*/
    
    /* Copy the control bit 4j, 4j+1 into 4j+2, 4j+3 resp. */
    x4 = _mm256_shuffle_epi32(x4, 0b01000100);    /* x4 = | 000 l6  000 l6  000 l4  000 l4  000 l2  000 l2  000 l0  000 l0  |*/
    x5 = _mm256_shuffle_epi32(x5, 0b01000100);    /* x5 = | 000 l14 000 l14 000 l12 000 l12 000 l10 000 l10 000 l8  000 l8  |*/
    x6 = _mm256_shuffle_epi32(x6, 0b01000100);    /* x6 = | 000 l22 000 l22 000 l20 000 l20 000 l18 000 l18 000 l16 000 l16 |*/
    x7 = _mm256_shuffle_epi32(x7, 0b01000100);    /* x7 = | 000 l30 000 l30 000 l28 000 l28 000 l26 000 l26 000 l24 000 l24 |*/

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL */
    x0 = _mm256_permutevar8x32_epi32(x0, x4 ^ id_32x8);     /* x4 = | ggg p7  ggg p6  ggg p5  ggg p4  ggg p3  ggg p2  ggg p1  ggg p0  |*/
    x1 = _mm256_permutevar8x32_epi32(x1, x5 ^ id_32x8);     /* x5 = | ggg p15 ggg p14 ggg p13 ggg p12 ggg p11 ggg p10 ggg p9  ggg p8  |*/
    x2 = _mm256_permutevar8x32_epi32(x2, x6 ^ id_32x8);     /* x6 = | ggg p23 ggg p22 ggg p21 ggg p20 ggg p19 ggg p18 ggg p17 ggg p16 |*/
    x3 = _mm256_permutevar8x32_epi32(x3, x7 ^ id_32x8);     /* x7 = | ggg p31 ggg p30 ggg p29 ggg p28 ggg p27 ggg p26 ggg p25 ggg p24 |*/

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    int32x8_store(&pi[x+32], x0);
    int32x8_store(&pi[x+48], x2);

    /* We accumulate control bits lj and save them in cb2. */
    x4 = _mm256_shuffle_epi32(x4, 0b11011000);
    x5 = _mm256_shuffle_epi32(x5, 0b11011000);
    x6 = _mm256_shuffle_epi32(x6, 0b11011000);
    x7 = _mm256_shuffle_epi32(x7, 0b11011000);
    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;
    x7 =  _mm256_slli_epi32(x7, 8);                       
    x6 ^= x7; 
    x6 <<= 16;                               
    x4 ^= x6;                                
    cbs2 = _mm256_shuffle_epi8(x4, cb_orderer);

    /* Join accumulated control bits. */
    x0 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    x1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00110001);
    x1 <<= 16;
    cbs1 = x0 ^ x1; 

    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 6);
    pos += 1;
  }

  /* ========= END of second iteration of loop where w = 1  ========= */
  /* ========= Start of third iteration of loop where w = 2 ========= */
  pos = 2 * (n >> 6);
  w = 2;
  jump = 4;
  
  jump32x8 <<= 1;
  jump16x16 <<= 1; 
  cb_orderer = _mm256_set_epi64x(0x0f0b07030e0a0602, 0x0d0905010c080400, 0x0f0b07030e0a0602, 0x0d0905010c080400);

  /* Set A = ( (pi[x]^4) << 16 ) + pi[x^4] */
  for (x=0; x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    
    /* We compose the permute 10110001 followed by usual 11011000 */
    x1 = _mm256_permute4x64_epi64(x0, 0b10001101);      
    
    x0 ^= jump16x16;                                    /* x0 = pi[x]^4 */
    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    
    x2 = _mm256_unpacklo_epi16(x1, x0);             
    x3 = _mm256_unpackhi_epi16(x1, x0);     
    int32x8_store(&A[x],   x2);
    int32x8_store(&A[x+8], x3);
  }
  
  int32_sort_interlaced(A,m,w); /* A = (id << 16) + pibar_2 */
  cyclemin_calc(pi, m, n, w, temp);

  /* Calculate and set f and F. */
  id = id_32x8;
  for (x = 0; x < n; x += 64) {
    /* Load in control bits. */
    x0 = int32x8_load(&B[x   ]);
    x1 = int32x8_load(&B[x+ 8]);
    x2 = int32x8_load(&B[x+16]);
    x3 = int32x8_load(&B[x+24]);

    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x   ]);
    x5 = int32x8_load(&A[x+ 8]);
    x6 = int32x8_load(&A[x+16]);
    x7 = int32x8_load(&A[x+24]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);
    /* Set A = (pi^-1 <<16) +  F */
    /* The shuffle copies the control bits  8j, 8j+1, 8j+2, 8j+3, into 8j+4, 8j+5, 8j+6, 8j+7 place resp. */
    x4 ^= _mm256_permute2x128_si256(x0, x0, 0);
    x5 ^= _mm256_permute2x128_si256(x1, x1, 0);
    x6 ^= _mm256_permute2x128_si256(x2, x2, 0);
    x7 ^= _mm256_permute2x128_si256(x3, x3, 0);

    int32x8_store(&A[x   ], x4);
    int32x8_store(&A[x+ 8], x5);
    int32x8_store(&A[x+16], x6);
    int32x8_store(&A[x+24], x7);

    id += id_32x8_incr;
    id += id_32x8_incr;
    id += id_32x8_incr;
    id += id_32x8_incr;
    

    /* We accumulate control bits and save them in cb1. */
    /* x0       = | -- -- -- 07 | -- -- -- 06 | -- -- -- 05 | -- -- -- 04 | -- -- -- 03 | -- -- -- 02 | -- -- -- 01 | -- -- -- 00 | */
    /* x1 <<  8 = | -- -- 15 -- | -- -- 14 -- | -- -- 13 -- | -- -- 12 -- | -- -- 11 -- | -- -- 10 -- | -- -- 09 -- | -- -- 08 -- | */
    /* x2 << 16 = | -- 23 -- -- | -- 22 -- -- | -- 21 -- -- | -- 20 -- -- | -- 19 -- -- | -- 18 -- -- | -- 17 -- -- | -- 16 -- -- | */
    /* x3 << 24 = | 31 -- -- -- | 30 -- -- -- | 29 -- -- -- | 28 -- -- -- | 27 -- -- -- | 26 -- -- -- | 25 -- -- -- | 24 -- -- -- | */

    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;
    cbs1 = x0 ^ x2;
    /*                                                                    |------>>   We want these cbs                             */
    /* cbs1     = | 31 23 15 07 | 30 22 14 06 | 29 21 13 05 | 28 20 12 04 | 27 19 11 03 | 26 18 10 02 | 25 17 09 01 | 24 16 08 00 | */
    
    /* Load in next set of control bits. */
    x0 = int32x8_load(&B[x+32]);
    x1 = int32x8_load(&B[x+40]);
    x2 = int32x8_load(&B[x+48]);
    x3 = int32x8_load(&B[x+56]);

    /* Get A = (id << 16) + pi^-1 */
    x4 = int32x8_load(&A[x+32]);
    x5 = int32x8_load(&A[x+40]);
    x6 = int32x8_load(&A[x+48]);
    x7 = int32x8_load(&A[x+56]);

    /* Set A = (pi^-1 <<16) + id */
    x4 = _mm256_shuffle_epi8(x4, swap_adjacents);
    x5 = _mm256_shuffle_epi8(x5, swap_adjacents);
    x6 = _mm256_shuffle_epi8(x6, swap_adjacents);
    x7 = _mm256_shuffle_epi8(x7, swap_adjacents);

    /* Copy the control bit 8j, 8j+1, 8j+2, 8j+3, into 8j+4, 8j+5, 8j+6, 8j+7 place resp. */
    x4 ^= _mm256_permute2x128_si256(x0, x0, 0);
    x5 ^= _mm256_permute2x128_si256(x1, x1, 0);
    x6 ^= _mm256_permute2x128_si256(x2, x2, 0);
    x7 ^= _mm256_permute2x128_si256(x3, x3, 0);

    int32x8_store(&A[x+32], x4);
    int32x8_store(&A[x+40], x5);
    int32x8_store(&A[x+48], x6);
    int32x8_store(&A[x+56], x7);

    /* We accumulate control bits and save them in cbs2. */
    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;
    cbs2 = x0 ^ x2;

    /*                                                                    |------>>   We want these cbs                             */
    /* cbs1     = | 31 23 15 07 | 30 22 14 06 | 29 21 13 05 | 28 20 12 04 | 27 19 11 03 | 26 18 10 02 | 25 17 09 01 | 24 16 08 00 | */
    /* cbs2     = | 63 55 47 39 | 62 54 46 38 | 61 53 45 37 | 60 52 44 36 | 59 51 43 35 | 58 50 42 34 | 57 49 41 33 | 56 48 40 32 | */
    cbs1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    cbs1 = _mm256_shuffle_epi8(cbs1, cb_orderer);

    /* Align to extract control bits with movemask. */
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 5);
    pos += 1;

  } 

  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(A,m,w);
  
  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-4) * (n >> 6);
  for (x = 0; x < n; x +=64) {
    x0 = int32x8_load(&A[x   ]);
    x1 = int32x8_load(&A[x+ 8]);
    x2 = int32x8_load(&A[x+16]);
    x3 = int32x8_load(&A[x+24]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;  
    x5 = x1 & jump32x8;  
    x6 = x2 & jump32x8;  
    x7 = x3 & jump32x8;  

    /* Copy the control bit 8j, 8j+1, 8j+2, 8j+3, into 8j+4, 8j+5, 8j+6, 8j+7 place resp. */
    x4 = _mm256_permute2x128_si256(x4, x4, 0);
    x5 = _mm256_permute2x128_si256(x5, x5, 0);
    x6 = _mm256_permute2x128_si256(x6, x6, 0);
    x7 = _mm256_permute2x128_si256(x7, x7, 0);

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL.  */
    x0 = _mm256_permutevar8x32_epi32(x0, x4^id_32x8);  
    x1 = _mm256_permutevar8x32_epi32(x1, x5^id_32x8);  
    x2 = _mm256_permutevar8x32_epi32(x2, x6^id_32x8);  
    x3 = _mm256_permutevar8x32_epi32(x3, x7^id_32x8);  

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    int32x8_store(&pi[x   ], x0);
    int32x8_store(&pi[x+16], x2);

    /* We accumulate control bits and save them in cb1. */
    /* x4       = | -- -- -- 07 | -- -- -- 06 | -- -- -- 05 | -- -- -- 04 | -- -- -- 03 | -- -- -- 02 | -- -- -- 01 | -- -- -- 00 | */
    /* x5 <<  8 = | -- -- 15 -- | -- -- 14 -- | -- -- 13 -- | -- -- 12 -- | -- -- 11 -- | -- -- 10 -- | -- -- 09 -- | -- -- 08 -- | */
    /* x6 << 16 = | -- 23 -- -- | -- 22 -- -- | -- 21 -- -- | -- 20 -- -- | -- 19 -- -- | -- 18 -- -- | -- 17 -- -- | -- 16 -- -- | */
    /* x7 << 24 = | 31 -- -- -- | 30 -- -- -- | 29 -- -- -- | 28 -- -- -- | 27 -- -- -- | 26 -- -- -- | 25 -- -- -- | 24 -- -- -- | */

    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;
    x7 = _mm256_slli_epi32(x7, 8);
    x6 ^= x7; 
    x6 <<= 16;
    cbs1 = x4 ^ x6;
    /*                                                                    |------>>   We want these cbs                             */
    /* cbs1     = | 31 23 15 07 | 30 22 14 06 | 29 21 13 05 | 28 20 12 04 | 27 19 11 03 | 26 18 10 02 | 25 17 09 01 | 24 16 08 00 | */

    
    x0 = int32x8_load(&A[x+32]);
    x1 = int32x8_load(&A[x+40]);
    x2 = int32x8_load(&A[x+48]);
    x3 = int32x8_load(&A[x+56]);

    /* Extract the control bits lj. */
    x4 = x0 & jump32x8;  
    x5 = x1 & jump32x8;  
    x6 = x2 & jump32x8;  
    x7 = x3 & jump32x8;  

    /* Copy the control bit 8j, 8j+1, 8j+2, 8j+3, into 8j+4, 8j+5, 8j+6, 8j+7 place resp. */
    x4 = _mm256_permute2x128_si256(x4, x4, 0);
    x5 = _mm256_permute2x128_si256(x5, x5, 0);
    x6 = _mm256_permute2x128_si256(x6, x6, 0);
    x7 = _mm256_permute2x128_si256(x7, x7, 0);

    /* Apply control bits, giving xi = (gibberish << 16) + FpiL.  */
    x0 = _mm256_permutevar8x32_epi32(x0, x4^id_32x8);  
    x1 = _mm256_permutevar8x32_epi32(x1, x5^id_32x8);  
    x2 = _mm256_permutevar8x32_epi32(x2, x6^id_32x8);  
    x3 = _mm256_permutevar8x32_epi32(x3, x7^id_32x8);  

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    int32x8_store(&pi[x+32], x0);
    int32x8_store(&pi[x+48], x2);

    x5 = _mm256_slli_epi32(x5, 8);
    x4 ^= x5;
    x7 = _mm256_slli_epi32(x7, 8);
    x6 ^= x7; 
    x6 <<= 16;
    cbs2 = x4 ^ x6;

    /*                                                                    |------>>   We want these cbs                             */
    /* cbs1     = | 31 23 15 07 | 30 22 14 06 | 29 21 13 05 | 28 20 12 04 | 27 19 11 03 | 26 18 10 02 | 25 17 09 01 | 24 16 08 00 | */
    /* cbs2     = | 63 55 47 39 | 62 54 46 38 | 61 53 45 37 | 60 52 44 36 | 59 51 43 35 | 58 50 42 34 | 57 49 41 33 | 56 48 40 32 | */
    cbs1 = _mm256_permute2x128_si256(cbs1, cbs2, 0b00100000);
    cbs1 = _mm256_shuffle_epi8(cbs1, cb_orderer);

    /* Align to extract control bits with movemask. */
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(cbs1 << 5);
    pos += 1;
  }

  /* =========   END of third iteration of loop where w = 2   ========= */
  /* ========= Start of fourth iteration of loop where w = 3  ========= */
  pos = 3 * (n >> 6);
  w = 3;
  jump = 8;
  jump32x8 <<= 1;
  jump16x16 <<= 1; 
  int32x8 cb_orderer_2 = _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0);
  int32x8 zero_32x8; zero_32x8 ^= zero_32x8; //= _mm256_set1_epi32(0);


  /* Set A = ( (pi[x]^8) << 16 ) + pi[x^8] */
  for (x=0; x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    /* We compose the permute 01001110 followed by usual 11011000 */
    x1 = _mm256_permute4x64_epi64(x0, 0b01110010);

    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    x0 ^= jump16x16;                                    /* x0 = pi[x]^8 */
    
    x2 = _mm256_unpacklo_epi16(x1, x0);             
    x3 = _mm256_unpackhi_epi16(x1, x0);     
    int32x8_store(&A[x],   x2);
    int32x8_store(&A[x+8], x3);
  }
  
  int32_sort_interlaced(A,m,w); /* Now A = (id << 16) + pibar_3 */
  cyclemin_calc(pi, m, n, w, temp);


  /* Calculate F and set A = (pi^-1 << 16) + F */
  for (x = 0; x < n; x += 64) {
    
    /* Load in control bits. */
    x0 = int32x8_load(&B[x   ]);
    x1 = int32x8_load(&B[x+16]);
    x2 = int32x8_load(&B[x+32]);
    x3 = int32x8_load(&B[x+48]);

    /* Load in A. */
    x4  = int32x8_load(&A[x   ]);
    x5  = int32x8_load(&A[x+ 8]);
    x6  = int32x8_load(&A[x+16]);
    x7  = int32x8_load(&A[x+24]);
    x8  = int32x8_load(&A[x+32]);
    x9  = int32x8_load(&A[x+40]);
    x10 = int32x8_load(&A[x+48]);
    x11 = int32x8_load(&A[x+56]);

    /* Set A = (pibar3 << 16) + F. */
    x4  = _mm256_shufflelo_epi16(x4 , 0b10110001); 
    x4  = _mm256_shufflehi_epi16(x4 , 0b10110001); 
    x5  = _mm256_shufflelo_epi16(x5 , 0b10110001); 
    x5  = _mm256_shufflehi_epi16(x5 , 0b10110001); 
    x6  = _mm256_shufflelo_epi16(x6 , 0b10110001); 
    x6  = _mm256_shufflehi_epi16(x6 , 0b10110001); 
    x7  = _mm256_shufflehi_epi16(x7 , 0b10110001);
    x7  = _mm256_shufflelo_epi16(x7 , 0b10110001);
    x8  = _mm256_shufflehi_epi16(x8 , 0b10110001); 
    x8  = _mm256_shufflelo_epi16(x8 , 0b10110001); 
    x9  = _mm256_shufflelo_epi16(x9 , 0b10110001); 
    x9  = _mm256_shufflehi_epi16(x9 , 0b10110001); 
    x10 = _mm256_shufflelo_epi16(x10, 0b10110001); 
    x10 = _mm256_shufflehi_epi16(x10, 0b10110001);  
    x11 = _mm256_shufflelo_epi16(x11, 0b10110001);  
    x11 = _mm256_shufflehi_epi16(x11, 0b10110001);  /* xi = (pibar3 << 16) + id */
    
    x4  ^= x0;
    x5  ^= x0;
    x6  ^= x1;
    x7  ^= x1;
    x8  ^= x2;
    x9  ^= x2;
    x10 ^= x3; 
    x11 ^= x3; /* xi = (pibar3 << 16) + F */

    int32x8_store(&A[x   ], x4); 
    int32x8_store(&A[x+ 8], x5); 
    int32x8_store(&A[x+16], x6); 
    int32x8_store(&A[x+24], x7);
    int32x8_store(&A[x+32], x8); 
    int32x8_store(&A[x+40], x9); 
    int32x8_store(&A[x+48], x10); 
    int32x8_store(&A[x+56], x11);

    /* We accumulate control bits and save them in x0. */
    /* x0       = | -- -- -- 07 | -- -- -- 06 | -- -- -- 05 | -- -- -- 04 | -- -- -- 03 | -- -- -- 02 | -- -- -- 01 | -- -- -- 00 | */
    /* x1 <<  8 = | -- -- 23 -- | -- -- 22 -- | -- -- 21 -- | -- -- 20 -- | -- -- 19 -- | -- -- 18 -- | -- -- 17 -- | -- -- 16 -- | */
    /* x2 << 16 = | -- 39 -- -- | -- 38 -- -- | -- 37 -- -- | -- 36 -- -- | -- 35 -- -- | -- 34 -- -- | -- 33 -- -- | -- 32 -- -- | */
    /* x3 << 24 = | 55 -- -- -- | 54 -- -- -- | 53 -- -- -- | 52 -- -- -- | 51 -- -- -- | 50 -- -- -- | 49 -- -- -- | 48 -- -- -- | */

    /* We need    | 55 54 53 52 | 51 50 49 48 | 39 38 37 36 | 35 34 33 32 | 23 22 21 20 | 19 18 17 16 | 07 06 05 04 | 03 02 01 00 | */

    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;          
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;          
    x0 ^= x2;/* = | 55 39 23 07 | 54 38 22 06 | 53 37 21 05 | 52 36 20 04 | 51 35 19 03 | 50 34 18 02 | 49 33 17 01 | 48 32 16 00 | */
    x0 = _mm256_shuffle_epi8(x0, cb_orderer);
    /* x0       = | 55 54 53 52 | 39 38 37 36 | 23 22 21 20 | 07 06 05 04 | 51 50 49 48 | 35 34 33 32 | 19 18 17 16 | 03 02 01 00 | */
    x0 = _mm256_permutevar8x32_epi32(x0, cb_orderer_2);
    /* x0       = | 55 54 53 52 | 51 50 49 48 | 39 38 37 36 | 35 34 33 32 | 23 22 21 20 | 19 18 17 16 | 07 06 05 04 | 03 02 01 00 | */

    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x0 << 4);
    pos += 1;
    
  }
  
  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(A,m,w);

  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-5) * (n >> 6);
  for (x = 0; x < n; x += 64) {
    x0 = int32x8_load(&A[x   ]);
    x1 = int32x8_load(&A[x+ 8]);
    x2 = int32x8_load(&A[x+16]);
    x3 = int32x8_load(&A[x+24]);
    x4 = int32x8_load(&A[x+32]);
    x5 = int32x8_load(&A[x+40]);
    x6 = int32x8_load(&A[x+48]);
    x7 = int32x8_load(&A[x+56]);

    /* Extract the control bits lj and shift down to 0th index. */
    x8  = (x0 & jump32x8) >> w;    
    x9  = (x2 & jump32x8) >> w;
    x10 = (x4 & jump32x8) >> w; 
    x11 = (x6 & jump32x8) >> w; 

    APPLY_CONTROL_BIT_32x8(x0, x1, x8 );
    APPLY_CONTROL_BIT_32x8(x2, x3, x9 );
    APPLY_CONTROL_BIT_32x8(x4, x5, x10);
    APPLY_CONTROL_BIT_32x8(x6, x7, x11);

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    VPCOMPRESSW_and_concat(x4, x5);
    VPCOMPRESSW_and_concat(x6, x7);

    int32x8_store(&pi[x   ], x0);
    int32x8_store(&pi[x+16], x2);
    int32x8_store(&pi[x+32], x4);
    int32x8_store(&pi[x+48], x6);

    /* We accumulate control bits and save them in x8. */
    /* x0       = | -- -- -- 07 | -- -- -- 06 | -- -- -- 05 | -- -- -- 04 | -- -- -- 03 | -- -- -- 02 | -- -- -- 01 | -- -- -- 00 | */
    /* x1 <<  8 = | -- -- 23 -- | -- -- 22 -- | -- -- 21 -- | -- -- 20 -- | -- -- 19 -- | -- -- 18 -- | -- -- 17 -- | -- -- 16 -- | */
    /* x2 << 16 = | -- 39 -- -- | -- 38 -- -- | -- 37 -- -- | -- 36 -- -- | -- 35 -- -- | -- 34 -- -- | -- 33 -- -- | -- 32 -- -- | */
    /* x3 << 24 = | 55 -- -- -- | 54 -- -- -- | 53 -- -- -- | 52 -- -- -- | 51 -- -- -- | 50 -- -- -- | 49 -- -- -- | 48 -- -- -- | */

    /* We need    | 55 54 53 52 | 51 50 49 48 | 39 38 37 36 | 35 34 33 32 | 23 22 21 20 | 19 18 17 16 | 07 06 05 04 | 03 02 01 00 | */

    x9 = _mm256_slli_epi32(x9, 8);
    x8 ^= x9;          
    x11 = _mm256_slli_epi32(x11, 8);
    x10 ^= x11; 
    x10 <<= 16;          
    x8 ^= x10;/* = | 55 39 23 07 | 54 38 22 06 | 53 37 21 05 | 52 36 20 04 | 51 35 19 03 | 50 34 18 02 | 49 33 17 01 | 48 32 16 00 | */
    x8 = _mm256_shuffle_epi8(x8, cb_orderer);
    /* x0       = | 55 54 53 52 | 39 38 37 36 | 23 22 21 20 | 07 06 05 04 | 51 50 49 48 | 35 34 33 32 | 19 18 17 16 | 03 02 01 00 | */
    x8 = _mm256_permutevar8x32_epi32(x8, cb_orderer_2);
    /* x0       = | 55 54 53 52 | 51 50 49 48 | 39 38 37 36 | 35 34 33 32 | 23 22 21 20 | 19 18 17 16 | 07 06 05 04 | 03 02 01 00 | */

    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x8 << 7);
    pos += 1;

  }
  
  /* =========  END of fourth iteration of loop where w = 3  ========= */
  /* ========= Start of fifth iteration of loop where w = 4  ========= */
  w += 1;
  pos = 4 * (n >> 6);
  jump <<= 1;
  jump32x8 <<= 1;
  jump16x16 <<= 1; 

  /* Set A = ( (pi[x]^(2**w)) << 16 ) + pi[x^(2**w)] */
  for (x=0; x < n; x += 16) {
    x0 = int32x8_load(&pi[x]);
    x1 = int32x8_load(&pi[x^jump]);
    x0 ^= jump16x16;
    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    x1 = _mm256_permute4x64_epi64(x1, 0b11011000);
    x2 = _mm256_unpacklo_epi16(x1, x0);             
    x3 = _mm256_unpackhi_epi16(x1, x0);     
    int32x8_store(&A[x],   x2);
    int32x8_store(&A[x+8], x3);
  }

  int32_sort_interlaced(A,m,w); /* Now A = (id << 16) + pibar_w */
  cyclemin_calc(pi, m, n, w, temp);

  /* Calculate and set f and F. */
  for (x = 0; x < n; x += 64) {
  
    /* Load in control bits. */
    x0 = int32x8_load(&B[x   ]);
    x1 = int32x8_load(&B[x+ 8]);
    x2 = int32x8_load(&B[x+32]);
    x3 = int32x8_load(&B[x+40]);

    /* Load in A. */
    x4  = int32x8_load(&A[x   ]);
    x5  = int32x8_load(&A[x+ 8]);
    x6  = int32x8_load(&A[x+16]);
    x7  = int32x8_load(&A[x+24]);
    x8  = int32x8_load(&A[x+32]);
    x9  = int32x8_load(&A[x+40]);
    x10 = int32x8_load(&A[x+48]);
    x11 = int32x8_load(&A[x+56]);

    /* Set A = (pibar4 << 16) + F. */
    x4  = _mm256_shufflelo_epi16(x4 , 0b10110001); 
    x4  = _mm256_shufflehi_epi16(x4 , 0b10110001); 
    x5  = _mm256_shufflelo_epi16(x5 , 0b10110001); 
    x5  = _mm256_shufflehi_epi16(x5 , 0b10110001); 
    x6  = _mm256_shufflelo_epi16(x6 , 0b10110001); 
    x6  = _mm256_shufflehi_epi16(x6 , 0b10110001); 
    x7  = _mm256_shufflelo_epi16(x7 , 0b10110001);
    x7  = _mm256_shufflehi_epi16(x7 , 0b10110001);
    x8  = _mm256_shufflelo_epi16(x8 , 0b10110001); 
    x8  = _mm256_shufflehi_epi16(x8 , 0b10110001); 
    x9  = _mm256_shufflelo_epi16(x9 , 0b10110001); 
    x9  = _mm256_shufflehi_epi16(x9 , 0b10110001); 
    x10 = _mm256_shufflelo_epi16(x10, 0b10110001); 
    x10 = _mm256_shufflehi_epi16(x10, 0b10110001);  
    x11 = _mm256_shufflelo_epi16(x11, 0b10110001);
    x11 = _mm256_shufflehi_epi16(x11, 0b10110001);  /* xi = (pibar4 << 16) + id */

    x4  ^= x0;
    x5  ^= x1;
    x6  ^= x0;
    x7  ^= x1;
    x8  ^= x2;
    x9  ^= x3;
    x10 ^= x2; 
    x11 ^= x3; /* xi = (pibar4 << 16) + F */

    int32x8_store(&A[x   ], x4); 
    int32x8_store(&A[x+ 8], x5); 
    int32x8_store(&A[x+16], x6); 
    int32x8_store(&A[x+24], x7);
    int32x8_store(&A[x+32], x8); 
    int32x8_store(&A[x+40], x9); 
    int32x8_store(&A[x+48], x10); 
    int32x8_store(&A[x+56], x11);

    /* We accumulate control bits and save them in x0. */
    x1 = _mm256_slli_epi32(x1, 8);
    x0 ^= x1;          
    x3 = _mm256_slli_epi32(x3, 8);
    x2 ^= x3; 
    x2 <<= 16;          
    x0 ^= x2;
    x0 = _mm256_shuffle_epi8(x0, cb_orderer);
    x0 = _mm256_permutevar8x32_epi32(x0, cb_orderer_2);
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x0 << 3);
    pos += 1;
  }

  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(A,m,w);
  
  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-w-2) * (n >> 6);
  for (x = 0; x < n; x += 64) {
    x0 = int32x8_load(&A[x   ]);
    x1 = int32x8_load(&A[x+ 8]);
    x2 = int32x8_load(&A[x+16]);
    x3 = int32x8_load(&A[x+24]);
    x4 = int32x8_load(&A[x+32]);
    x5 = int32x8_load(&A[x+40]);
    x6 = int32x8_load(&A[x+48]);
    x7 = int32x8_load(&A[x+56]);

    /* Extract the control bits lj and shift down to 0th index. */
    x8  = (x0 & jump32x8) >> w;    
    x9  = (x1 & jump32x8) >> w;
    x10 = (x4 & jump32x8) >> w; 
    x11 = (x5 & jump32x8) >> w;

    APPLY_CONTROL_BIT_32x8(x0, x2, x8 );
    APPLY_CONTROL_BIT_32x8(x1, x3, x9 );
    APPLY_CONTROL_BIT_32x8(x4, x6, x10);
    APPLY_CONTROL_BIT_32x8(x5, x7, x11);

    /* Now extract FpiL from int32* A into int16* pi. */
    VPCOMPRESSW_and_concat(x0, x1);
    VPCOMPRESSW_and_concat(x2, x3);
    VPCOMPRESSW_and_concat(x4, x5);
    VPCOMPRESSW_and_concat(x6, x7);

    int32x8_store(&pi[x   ], x0);
    int32x8_store(&pi[x+16], x2);
    int32x8_store(&pi[x+32], x4);
    int32x8_store(&pi[x+48], x6);

    x9 = _mm256_slli_epi32(x9, 8);
    x8 ^= x9;          
    x11 = _mm256_slli_epi32(x11, 8);
    x10 ^= x11; 
    x10 <<= 16;          
    x8 ^= x10;
    x8 = _mm256_shuffle_epi8(x8, cb_orderer);
    x8 = _mm256_permutevar8x32_epi32(x8, cb_orderer_2);
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x8 << 7);
    pos += 1;
  }

  /* =========  END of fifth iteration of loop where w = 4  ========= */
  /* ========= Start of fifth iteration of loop where w = 5  ========= */
  if ( m > 6) {
    w += 1;
    pos = w * (n >> 6);
    jump <<= 1;
    jump32x8 <<= 1;
    jump16x16 <<= 1;

    /* Set A = ( (pi[x]^(2**w)) << 16 ) + pi[x^(2**w)] */
    for (x=0; x < n; x += 16) {
      x0 = int32x8_load(&pi[x]);
      x1 = int32x8_load(&pi[x^jump]);
      x0 ^= jump16x16;
      x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
      x1 = _mm256_permute4x64_epi64(x1, 0b11011000);
      x2 = _mm256_unpacklo_epi16(x1, x0);             
      x3 = _mm256_unpackhi_epi16(x1, x0);     
      int32x8_store(&A[x],   x2);
      int32x8_store(&A[x+8], x3);
    }


    int32_sort_interlaced(A,m,w); /* Now A = (id << 16) + pibar_w */
    cyclemin_calc(pi, m, n, w, temp);



    /* Calculate and set f and F. */
    for (x = 0; x < n; x += 64) {
    
      /* Load in control bits. */
      x0 = int32x8_load(&B[x   ]);
      x1 = int32x8_load(&B[x+ 8]);
      x2 = int32x8_load(&B[x+16]);
      x3 = int32x8_load(&B[x+24]);

      /* Load in A. */
      x4  = int32x8_load(&A[x   ]);
      x5  = int32x8_load(&A[x+ 8]);
      x6  = int32x8_load(&A[x+16]);
      x7  = int32x8_load(&A[x+24]);
      x8  = int32x8_load(&A[x+32]);
      x9  = int32x8_load(&A[x+40]);
      x10 = int32x8_load(&A[x+48]);
      x11 = int32x8_load(&A[x+56]);

      /* Set A = (pibar4 << 16) + F. */
      x4  = _mm256_shufflelo_epi16(x4 , 0b10110001); 
      x5  = _mm256_shufflelo_epi16(x5 , 0b10110001); 
      x6  = _mm256_shufflelo_epi16(x6 , 0b10110001); 
      x7  = _mm256_shufflelo_epi16(x7 , 0b10110001);
      x8  = _mm256_shufflelo_epi16(x8 , 0b10110001); 
      x9  = _mm256_shufflelo_epi16(x9 , 0b10110001); 
      x10 = _mm256_shufflelo_epi16(x10, 0b10110001); 
      x11 = _mm256_shufflelo_epi16(x11, 0b10110001);
      x4  = _mm256_shufflehi_epi16(x4 , 0b10110001); 
      x5  = _mm256_shufflehi_epi16(x5 , 0b10110001); 
      x6  = _mm256_shufflehi_epi16(x6 , 0b10110001); 
      x7  = _mm256_shufflehi_epi16(x7 , 0b10110001);
      x8  = _mm256_shufflehi_epi16(x8 , 0b10110001); 
      x9  = _mm256_shufflehi_epi16(x9 , 0b10110001); 
      x10 = _mm256_shufflehi_epi16(x10, 0b10110001);  /* For 4 <= i <= 11, */
      x11 = _mm256_shufflehi_epi16(x11, 0b10110001);  /* xi = (pibar5 << 16) + id */

      x4  ^= x0;
      x5  ^= x1;
      x6  ^= x2;
      x7  ^= x3;
      x8  ^= x0;
      x9  ^= x1;
      x10 ^= x2; /* For 4 <= i <= 11, */
      x11 ^= x3; /* xi = (pibar5 << 16) + F */

      int32x8_store(&A[x   ], x4); 
      int32x8_store(&A[x+ 8], x5); 
      int32x8_store(&A[x+16], x6); 
      int32x8_store(&A[x+24], x7);
      int32x8_store(&A[x+32], x8); 
      int32x8_store(&A[x+40], x9); 
      int32x8_store(&A[x+48], x10); 
      int32x8_store(&A[x+56], x11);

      /* We accumulate control bits and save them in x0. */
      x1 = _mm256_slli_epi32(x1, 8);
      x0 ^= x1;          
      x3 = _mm256_slli_epi32(x3, 8);
      x2 ^= x3; 
      x2 <<= 16;          
      x0 ^= x2;
      x0 = _mm256_shuffle_epi8(x0, cb_orderer);
      x0 = _mm256_permutevar8x32_epi32(x0, cb_orderer_2);
      ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x0 << (7-w));
      pos += 1;
    
    }

    
    /* Set B = (id<<16) + F(pi) */
    int32_sort_interlaced(A,m,w);
    
    /* Calculate l and set pi = M = FpiL. */
    pos = (2*m-w-2) * (n >> 6);

    for (x = 0; x < n; x += 64) {
      x0 = int32x8_load(&A[x   ]);
      x1 = int32x8_load(&A[x+ 8]);
      x2 = int32x8_load(&A[x+16]);
      x3 = int32x8_load(&A[x+24]);
      x4 = int32x8_load(&A[x+32]);
      x5 = int32x8_load(&A[x+40]);
      x6 = int32x8_load(&A[x+48]);
      x7 = int32x8_load(&A[x+56]);

      /* Extract the control bits lj and shift down to 0th index. */
      x8  = (x0 & jump32x8) >> w;    
      x9  = (x1 & jump32x8) >> w;
      x10 = (x2 & jump32x8) >> w; 
      x11 = (x3 & jump32x8) >> w;

      APPLY_CONTROL_BIT_32x8(x0, x4, x8 );
      APPLY_CONTROL_BIT_32x8(x1, x5, x9 );
      APPLY_CONTROL_BIT_32x8(x2, x6, x10);
      APPLY_CONTROL_BIT_32x8(x3, x7, x11);

      /* Now extract FpiL from int32* A into int16* pi. */
      VPCOMPRESSW_and_concat(x0, x1);
      VPCOMPRESSW_and_concat(x2, x3);
      VPCOMPRESSW_and_concat(x4, x5);
      VPCOMPRESSW_and_concat(x6, x7);

      int32x8_store(&pi[x   ], x0);
      int32x8_store(&pi[x+16], x2);
      int32x8_store(&pi[x+32], x4);
      int32x8_store(&pi[x+48], x6);

      x9 = _mm256_slli_epi32(x9, 8);
      x8 ^= x9;          
      x11 = _mm256_slli_epi32(x11, 8);
      x10 ^= x11; 
      x10 <<= 16;          
      x8 ^= x10;
      x8 = _mm256_shuffle_epi8(x8, cb_orderer);
      x8 = _mm256_permutevar8x32_epi32(x8, cb_orderer_2);
      ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x8 << 7);
      pos += 1;

    }
  }

  for (w = 6; w < m-1; w++) {
    pos = w * (n >> 6);
    jump <<= 1;
    jump32x8 <<= 1;
    jump16x16 <<= 1; 


    /* w >= 6 */
    /* Set A = ( (pi[x]^(2**w)) << 16 ) + pi[x^(2**w)] */
    for (x=0; x < n; x += 16) {
      x0 = int32x8_load(&pi[x]);
      x1 = int32x8_load(&pi[x^jump]);
      x0 ^= jump16x16;
      x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
      x1 = _mm256_permute4x64_epi64(x1, 0b11011000);
      x2 = _mm256_unpacklo_epi16(x1, x0);             
      x3 = _mm256_unpackhi_epi16(x1, x0);     
      int32x8_store(&A[x],   x2);
      int32x8_store(&A[x+8], x3);
    }


    int32_sort_interlaced(A,m,w); /* Now A = (id << 16) + pibar_w */
    cyclemin_calc(pi, m, n, w, temp);

    /* Calculate and set f and F. */
    for (j=0; j < n; j += 2*jump) {
      for (k=0; k < jump; k+=32) {
        /* Load in control bits. */
        x0 = int32x8_load(&B[j+k   ]);
        x1 = int32x8_load(&B[j+k+ 8]);
        x2 = int32x8_load(&B[j+k+16]);
        x3 = int32x8_load(&B[j+k+24]);
      
        /* Load in A. */
        x4  = int32x8_load(&A[j+k   ]);
        x5  = int32x8_load(&A[j+k+ 8]);
        x6  = int32x8_load(&A[j+k+16]);
        x7  = int32x8_load(&A[j+k+24]);
        x8  = int32x8_load(&A[j+k+jump   ]);
        x9  = int32x8_load(&A[j+k+jump+ 8]);
        x10 = int32x8_load(&A[j+k+jump+16]);
        x11 = int32x8_load(&A[j+k+jump+24]);

        /* Set A = (pibar4 << 16) + F. */
        x4  = _mm256_shufflelo_epi16(x4 , 0b10110001); 
        x5  = _mm256_shufflelo_epi16(x5 , 0b10110001); 
        x6  = _mm256_shufflelo_epi16(x6 , 0b10110001); 
        x7  = _mm256_shufflelo_epi16(x7 , 0b10110001);
        x8  = _mm256_shufflelo_epi16(x8 , 0b10110001); 
        x9  = _mm256_shufflelo_epi16(x9 , 0b10110001); 
        x10 = _mm256_shufflelo_epi16(x10, 0b10110001); 
        x11 = _mm256_shufflelo_epi16(x11, 0b10110001);
        x4  = _mm256_shufflehi_epi16(x4 , 0b10110001); 
        x5  = _mm256_shufflehi_epi16(x5 , 0b10110001); 
        x6  = _mm256_shufflehi_epi16(x6 , 0b10110001); 
        x7  = _mm256_shufflehi_epi16(x7 , 0b10110001);
        x8  = _mm256_shufflehi_epi16(x8 , 0b10110001); 
        x9  = _mm256_shufflehi_epi16(x9 , 0b10110001); 
        x10 = _mm256_shufflehi_epi16(x10, 0b10110001);  /* For 4 <= i <= 11, */
        x11 = _mm256_shufflehi_epi16(x11, 0b10110001);  /* xi = (pibarw << 16) + id */

        x4  ^= x0;
        x5  ^= x1;
        x6  ^= x2;
        x7  ^= x3;
        x8  ^= x0;
        x9  ^= x1;
        x10 ^= x2; /* For 4 <= i <= 11, */
        x11 ^= x3; /* xi = (pibarw << 16) + F */

        
        int32x8_store(&A[j+k   ], x4); 
        int32x8_store(&A[j+k+ 8], x5); 
        int32x8_store(&A[j+k+16], x6); 
        int32x8_store(&A[j+k+24], x7);
        int32x8_store(&A[j+k+jump   ], x8); 
        int32x8_store(&A[j+k+jump+ 8], x9); 
        int32x8_store(&A[j+k+jump+16], x10); 
        int32x8_store(&A[j+k+jump+24], x11);

        /* We accumulate control bits and save them in x0. We apply extra shifts to keep the w>7 cases
           in the correct byte-lanes for the accumulation below. */
        x0 >>= w;
        x1 >>= w;
        x2 >>= w;
        x3 >>= w;
        
        x1 = _mm256_slli_epi32(x1, 8);
        x0 ^= x1;          
        x3 = _mm256_slli_epi32(x3, 8);
        x2 ^= x3; 
        x2 <<= 16;          
        x0 ^= x2;
        x0 = _mm256_shuffle_epi8(x0, cb_orderer);
        x0 = _mm256_permutevar8x32_epi32(x0, cb_orderer_2);
        ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x0 << 7);
        pos += 1;
        
      }
    }

    /* Set B = (id<<16) + F(pi) */
    int32_sort_interlaced(A,m,w);

    /* Calculate l and set pi = M = FpiL. */
    pos = (2*m-w-2) * (n >> 6);
    for (j=0; j < n; j += 2*jump) {
      for (k=0; k < jump; k+=32) {
        x0 = int32x8_load(&A[j+k   ]);
        x1 = int32x8_load(&A[j+k+ 8]);
        x2 = int32x8_load(&A[j+k+16]);
        x3 = int32x8_load(&A[j+k+24]);
        x4 = int32x8_load(&A[j+k+jump   ]);
        x5 = int32x8_load(&A[j+k+jump+ 8]);
        x6 = int32x8_load(&A[j+k+jump+16]);
        x7 = int32x8_load(&A[j+k+jump+24]);

        /* Extract the control bits lj and shift down to 0th index. */
        x8  = (x0 & jump32x8) >> w;    
        x9  = (x1 & jump32x8) >> w;
        x10 = (x2 & jump32x8) >> w; 
        x11 = (x3 & jump32x8) >> w;

        APPLY_CONTROL_BIT_32x8(x0, x4, x8 );
        APPLY_CONTROL_BIT_32x8(x1, x5, x9 );
        APPLY_CONTROL_BIT_32x8(x2, x6, x10);
        APPLY_CONTROL_BIT_32x8(x3, x7, x11);

        /* Now extract FpiL from int32* A into int16* pi. */
        VPCOMPRESSW_and_concat(x0, x1);
        VPCOMPRESSW_and_concat(x2, x3);
        VPCOMPRESSW_and_concat(x4, x5);
        VPCOMPRESSW_and_concat(x6, x7);
        
        int32x8_store(&pi[j+k   ], x0);
        int32x8_store(&pi[j+k+16], x2);
        int32x8_store(&pi[j+k+jump], x4);
        int32x8_store(&pi[j+k+jump+16], x6);

        x9 = _mm256_slli_epi32(x9, 8);
        x8 ^= x9;          
        x11 = _mm256_slli_epi32(x11, 8);
        x10 ^= x11; 
        x10 <<= 16;          
        x8 ^= x10;
        x8 = _mm256_shuffle_epi8(x8, cb_orderer);
        x8 = _mm256_permutevar8x32_epi32(x8, cb_orderer_2);
        ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x8 << 7);
        pos += 1;
      }
    }
  }


  /* The final control bits apply a transposition of the form (x, x^2^(m-1)) 
    Therefore, if the (m-1)th bit of pi[x] is 1, our control bit is one.     */
  pos = (m-1) * (n >> 6);
  jump16x16 <<= 1;        
  cb_orderer = _mm256_set_epi64x(0x0f0d0b0907050301, 0x0e0c0a0806040200, 0x0f0d0b0907050301, 0x0e0c0a0806040200);

  /* 0x0f0d0b0907050301, 0x0e0c0a0806040200 */
  for (j =0; j < n/2; j+=32) {
    x0 = int32x8_load(&pi[j   ]);
    x1 = int32x8_load(&pi[j+16]);

    x0 = (x0 & jump16x16) >> (m-1);
    x1 = ((x1 & jump16x16) >> (m-1)) << 8;

    x0 ^= x1;
    x0 = _mm256_shuffle_epi8(x0, cb_orderer);
    x0 = _mm256_permute4x64_epi64(x0, 0b11011000);
    
    ( (int32_t *) out)[pos] = _mm256_movemask_epi8(x0 << 7);
    pos+=1;
  }
}
#endif