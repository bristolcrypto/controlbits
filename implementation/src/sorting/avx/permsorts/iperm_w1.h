#include "../../../utilities/utilities.h"

static void iperm_w1(int32_t *x, int32_t m);


/* 
    Scroll down to function iperm_w1 for the description. 
    In the 'large mask' functions, we complement lanes to avoid xoring in the masks.
 */

static inline void final_3_layers_small_mask_w1(int32_t *x, int32_t m, int32x8 mask) {
  /* The final three layers have comparators of widths 8, n/4, n/2. */
  int32_t q = 1 << (m-2);
  for (int32_t i = 0; i < q; i += 16) {
    int32x8 x0 = int32x8_load(&x[i      ]);
    int32x8 x1 = int32x8_load(&x[i    +8]);
    int32x8 x2 = int32x8_load(&x[i+1*q  ]);
    int32x8 x3 = int32x8_load(&x[i+1*q+8]);
    int32x8 x4 = int32x8_load(&x[i+2*q  ]);
    int32x8 x5 = int32x8_load(&x[i+2*q+8]);
    int32x8 x6 = int32x8_load(&x[i+3*q  ]);
    int32x8 x7 = int32x8_load(&x[i+3*q+8]);
    
    /* Layer of width 8. */
    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    /* Layer of width n/4. */
    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    /* Layer of width n/2. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    x0 ^= mask;
    x1 ^= mask;
    x2 ^= mask;
    x3 ^= mask;
    x4 ^= mask;
    x5 ^= mask;
    x6 ^= mask;
    x7 ^= mask;

    int32x8_store(&x[i      ], x0);
    int32x8_store(&x[i    +8], x1);
    int32x8_store(&x[i+1*q  ], x2);
    int32x8_store(&x[i+1*q+8], x3);
    int32x8_store(&x[i+2*q  ], x4);
    int32x8_store(&x[i+2*q+8], x5);
    int32x8_store(&x[i+3*q  ], x6);
    int32x8_store(&x[i+3*q+8], x7);
  }
}

static inline void final_3_layers_large_mask_w1(int32_t *x, int32_t m, int32_t mask) {
  
  int32_t q = 1 << (m - 2);

  /* The mask is atleast of size 32, at most of size n / 8 */
  for (int b = 0; b < q; b += 2*mask) {
    for (int i = 0; i < mask; i += 16) {
      int32x8 x0 = int32x8_load(&x[b+i      ]);
      int32x8 x1 = int32x8_load(&x[b+i    +8]);
      int32x8 x2 = int32x8_load(&x[b+i+1*q  ]);
      int32x8 x3 = int32x8_load(&x[b+i+1*q+8]);
      int32x8 x4 = int32x8_load(&x[b+i+2*q  ]);
      int32x8 x5 = int32x8_load(&x[b+i+2*q+8]);
      int32x8 x6 = int32x8_load(&x[b+i+3*q  ]);
      int32x8 x7 = int32x8_load(&x[b+i+3*q+8]);

      /* Layer of width 8. */
      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3);
      int32x8_MINMAX(x4, x5);
      int32x8_MINMAX(x6, x7);

      /* Layer of width q. */
      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7);

      /* Layer of width 2q. */
      int32x8_MINMAX(x0, x4);
      int32x8_MINMAX(x1, x5);
      int32x8_MINMAX(x2, x6);
      int32x8_MINMAX(x3, x7);

      int32x8_store(&x[b+i      ], x0);
      int32x8_store(&x[b+i    +8], x1);
      int32x8_store(&x[b+i+1*q  ], x2);
      int32x8_store(&x[b+i+1*q+8], x3);
      int32x8_store(&x[b+i+2*q  ], x4);
      int32x8_store(&x[b+i+2*q+8], x5);
      int32x8_store(&x[b+i+3*q  ], x6);
      int32x8_store(&x[b+i+3*q+8], x7);

      /* Same again on masked elements. */
      x0 = int32x8_load(&x[b+i+mask      ]);
      x1 = int32x8_load(&x[b+i+mask    +8]);
      x2 = int32x8_load(&x[b+i+mask+1*q  ]);
      x3 = int32x8_load(&x[b+i+mask+1*q+8]);
      x4 = int32x8_load(&x[b+i+mask+2*q  ]);
      x5 = int32x8_load(&x[b+i+mask+2*q+8]);
      x6 = int32x8_load(&x[b+i+mask+3*q  ]);
      x7 = int32x8_load(&x[b+i+mask+3*q+8]);

      /* Layer of width 8. */
      int32x8_MINMAX(x1, x0);
      int32x8_MINMAX(x3, x2);
      int32x8_MINMAX(x5, x4);
      int32x8_MINMAX(x7, x6);
      /* Layer of width q. */
      int32x8_MINMAX(x2, x0);
      int32x8_MINMAX(x3, x1);
      int32x8_MINMAX(x6, x4);
      int32x8_MINMAX(x7, x5);
      /* Layer of width 2q. */
      int32x8_MINMAX(x4, x0);
      int32x8_MINMAX(x5, x1);
      int32x8_MINMAX(x6, x2);
      int32x8_MINMAX(x7, x3);

      int32x8_store(&x[b+i+mask      ], x0);
      int32x8_store(&x[b+i+mask    +8], x1);
      int32x8_store(&x[b+i+mask+1*q  ], x2);
      int32x8_store(&x[b+i+mask+1*q+8], x3);
      int32x8_store(&x[b+i+mask+2*q  ], x4);
      int32x8_store(&x[b+i+mask+2*q+8], x5);
      int32x8_store(&x[b+i+mask+3*q  ], x6);
      int32x8_store(&x[b+i+mask+3*q+8], x7);
    }
  }
}

static inline void greedy_3_layers_large_mask_w1(int32_t *x, int32_t m, int32_t start, int32_t mask) {
  /* In stage 2^k, the maximum comparison width 'u' is 2^k and mask is size 2^(k+1). Since the mask
     is greater than the comparison width, */
  int32_t n = 1 << m;
  int32_t u = start;
  int32_t l;
  while (u >> 2 >= 16) {
    /* This is only reached when the mask alternating period is >= 128 and u >= 64*/
    l = u >> 2;
    for (int b = 0; b < n; b += 2*mask) {
      for (int c = 0; c < mask; c += 2*u) {
        for (int i = 0; i < l; i += 8) {
          int32x8 x0 = int32x8_load(&x[b+c+i    ]);
          int32x8 x1 = int32x8_load(&x[b+c+i+1*l]);
          int32x8 x2 = int32x8_load(&x[b+c+i+2*l]);
          int32x8 x3 = int32x8_load(&x[b+c+i+3*l]);
          int32x8 x4 = int32x8_load(&x[b+c+i+4*l]);
          int32x8 x5 = int32x8_load(&x[b+c+i+5*l]);
          int32x8 x6 = int32x8_load(&x[b+c+i+6*l]);
          int32x8 x7 = int32x8_load(&x[b+c+i+7*l]);

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

          int32x8_store(&x[b+c+i    ], x0);
          int32x8_store(&x[b+c+i+1*l], x1);
          int32x8_store(&x[b+c+i+2*l], x2);
          int32x8_store(&x[b+c+i+3*l], x3);
          int32x8_store(&x[b+c+i+4*l], x4);
          int32x8_store(&x[b+c+i+5*l], x5);
          int32x8_store(&x[b+c+i+6*l], x6);
          int32x8_store(&x[b+c+i+7*l], x7);

          x0 = int32x8_load(&x[b+c+i    +mask]);
          x1 = int32x8_load(&x[b+c+i+1*l+mask]);
          x2 = int32x8_load(&x[b+c+i+2*l+mask]);
          x3 = int32x8_load(&x[b+c+i+3*l+mask]);
          x4 = int32x8_load(&x[b+c+i+4*l+mask]);
          x5 = int32x8_load(&x[b+c+i+5*l+mask]);
          x6 = int32x8_load(&x[b+c+i+6*l+mask]);
          x7 = int32x8_load(&x[b+c+i+7*l+mask]);

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

          int32x8_store(&x[b+c+i    +mask], x0);
          int32x8_store(&x[b+c+i+1*l+mask], x1);
          int32x8_store(&x[b+c+i+2*l+mask], x2);
          int32x8_store(&x[b+c+i+3*l+mask], x3);
          int32x8_store(&x[b+c+i+4*l+mask], x4);
          int32x8_store(&x[b+c+i+5*l+mask], x5);
          int32x8_store(&x[b+c+i+6*l+mask], x6);
          int32x8_store(&x[b+c+i+7*l+mask], x7);


        }
      }
    }
    u = l >> 1;
  }

  if ( u == 16 ) {
    /* Smallest mask is 32, so 2*smallest mask is 64. So we can iterate in
       chunks of 32. */
    for (int b = 0; b < n; b += 2 * mask)
      for (int c = 0; c < mask; c += 32) {
        int32x8 x0 = int32x8_load(&x[b+c   ]);
        int32x8 x1 = int32x8_load(&x[b+c+8 ]);
        int32x8 x2 = int32x8_load(&x[b+c+16]);
        int32x8 x3 = int32x8_load(&x[b+c+24]);
        int32x8 x4 = int32x8_load(&x[b+c+mask   ]);
        int32x8 x5 = int32x8_load(&x[b+c+mask+8 ]);
        int32x8 x6 = int32x8_load(&x[b+c+mask+16]);
        int32x8 x7 = int32x8_load(&x[b+c+mask+24]);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_store(&x[b+c   ],      x0);
        int32x8_store(&x[b+c+8 ],      x1);
        int32x8_store(&x[b+c+16],      x2);
        int32x8_store(&x[b+c+24],      x3);
        int32x8_store(&x[b+c+mask   ], x4);
        int32x8_store(&x[b+c+mask+8 ], x5);
        int32x8_store(&x[b+c+mask+16], x6);
        int32x8_store(&x[b+c+mask+24], x7);
    }
  } else if ( u == 32 )
  {
    /* Smallest mask is 32. On that stage, however, only u == 16 will be triggered. 
       So, smallest mask is 64 when this condition can be met.
    */
    for (int b = 0; b < n; b += 2 * mask) {
      for (int c = 0; c < mask; c += 64 ) {
        int32x8 x0 = int32x8_load(&x[b+c   ]);
        int32x8 x1 = int32x8_load(&x[b+c+8 ]);
        int32x8 x2 = int32x8_load(&x[b+c+16]);
        int32x8 x3 = int32x8_load(&x[b+c+24]);
        int32x8 x4 = int32x8_load(&x[b+c+32]);
        int32x8 x5 = int32x8_load(&x[b+c+40]);
        int32x8 x6 = int32x8_load(&x[b+c+48]);
        int32x8 x7 = int32x8_load(&x[b+c+56]);

        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);

        int32x8_store(&x[b+c   ], x0);
        int32x8_store(&x[b+c+8 ], x1);
        int32x8_store(&x[b+c+16], x2);
        int32x8_store(&x[b+c+24], x3);
        int32x8_store(&x[b+c+32], x4);
        int32x8_store(&x[b+c+40], x5);
        int32x8_store(&x[b+c+48], x6);
        int32x8_store(&x[b+c+56], x7);

        x0 = int32x8_load(&x[b+c+mask   ]);
        x1 = int32x8_load(&x[b+c+mask+8 ]);
        x2 = int32x8_load(&x[b+c+mask+16]);
        x3 = int32x8_load(&x[b+c+mask+24]);
        x4 = int32x8_load(&x[b+c+mask+32]);
        x5 = int32x8_load(&x[b+c+mask+40]);
        x6 = int32x8_load(&x[b+c+mask+48]);
        x7 = int32x8_load(&x[b+c+mask+56]);

        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);

        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_store(&x[b+c+mask   ], x0);
        int32x8_store(&x[b+c+mask+8 ], x1);
        int32x8_store(&x[b+c+mask+16], x2);
        int32x8_store(&x[b+c+mask+24], x3);
        int32x8_store(&x[b+c+mask+32], x4);
        int32x8_store(&x[b+c+mask+40], x5);
        int32x8_store(&x[b+c+mask+48], x6);
        int32x8_store(&x[b+c+mask+56], x7);


      }
    }
    
  }
  
}

static inline void greedy_3_layers_small_mask_w1(int32_t *x, int32_t m, int32_t start) {
  /* Applies the decreasing chain of sorts from start to 16 inclusive. 
     REQUIRES: n >= 64. We greedy apply three layers at a time to reduce
    memory overhead. */
  int32_t n = 1 << m;
  int32_t u = start;
  int32_t l;
  while (u >> 2 >= 16) {
    l = u >> 2;
    for (int b = 0; b < n; b += 2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[b+i    ]);
        int32x8 x1 = int32x8_load(&x[b+i+1*l]);
        int32x8 x2 = int32x8_load(&x[b+i+2*l]);
        int32x8 x3 = int32x8_load(&x[b+i+3*l]);
        int32x8 x4 = int32x8_load(&x[b+i+4*l]);
        int32x8 x5 = int32x8_load(&x[b+i+5*l]);
        int32x8 x6 = int32x8_load(&x[b+i+6*l]);
        int32x8 x7 = int32x8_load(&x[b+i+7*l]);

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

        int32x8_store(&x[b+i    ], x0);
        int32x8_store(&x[b+i+1*l], x1);
        int32x8_store(&x[b+i+2*l], x2);
        int32x8_store(&x[b+i+3*l], x3);
        int32x8_store(&x[b+i+4*l], x4);
        int32x8_store(&x[b+i+5*l], x5);
        int32x8_store(&x[b+i+6*l], x6);
        int32x8_store(&x[b+i+7*l], x7);
      }
    }
    u = l >> 1;
  }

  if ( u == 16 ) {
    for (int b = 0; b < n; b += 64) {
      int32x8 x0 = int32x8_load(&x[b   ]);
      int32x8 x1 = int32x8_load(&x[b+8 ]);
      int32x8 x2 = int32x8_load(&x[b+16]);
      int32x8 x3 = int32x8_load(&x[b+24]);
      int32x8 x4 = int32x8_load(&x[b+32]);
      int32x8 x5 = int32x8_load(&x[b+40]);
      int32x8 x6 = int32x8_load(&x[b+48]);
      int32x8 x7 = int32x8_load(&x[b+56]);

      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7);

      int32x8_store(&x[b   ], x0);
      int32x8_store(&x[b+8 ], x1);
      int32x8_store(&x[b+16], x2);
      int32x8_store(&x[b+24], x3);
      int32x8_store(&x[b+32], x4);
      int32x8_store(&x[b+40], x5);
      int32x8_store(&x[b+48], x6);
      int32x8_store(&x[b+56], x7);
    }
  }
  else if ( u == 32) {
    for (int b = 0; b < n; b += 64) {
      int32x8 x0 = int32x8_load(&x[b   ]);
      int32x8 x1 = int32x8_load(&x[b+8 ]);
      int32x8 x2 = int32x8_load(&x[b+16]);
      int32x8 x3 = int32x8_load(&x[b+24]);
      int32x8 x4 = int32x8_load(&x[b+32]);
      int32x8 x5 = int32x8_load(&x[b+40]);
      int32x8 x6 = int32x8_load(&x[b+48]);
      int32x8 x7 = int32x8_load(&x[b+56]);

      int32x8_MINMAX(x0, x4);
      int32x8_MINMAX(x1, x5);
      int32x8_MINMAX(x2, x6);
      int32x8_MINMAX(x3, x7);

      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7);

      int32x8_store(&x[b   ], x0);
      int32x8_store(&x[b+8 ], x1);
      int32x8_store(&x[b+16], x2);
      int32x8_store(&x[b+24], x3);
      int32x8_store(&x[b+32], x4);
      int32x8_store(&x[b+40], x5);
      int32x8_store(&x[b+48], x6);
      int32x8_store(&x[b+56], x7);
    }
  } 
}

static void iperm_w1(int32_t *x, int32_t m) {

  /* A description of the mapping applied to the comparator network
     is as follows:
    
    +-------------------+    +----------------------------+
    | comparator widths |    |   mask alternating period  |
    +----------+--------+    +----------------------------+
    | original | mapped |    | stage | original | mappped |
    +----------+--------+    +-------+----------+---------+
    |  2       | n / 2  |    | n / 2 |  4       |  n / 4  |   -+-
    |  4       | n / 4  |    +-------+----------+---------+    | Prelude
    +----------+--------+    | n / 4 |  8       |  8      |    | ( Stages n/2, n/4, 8 )
    |  8       | 8      |    |   8   |  16      |  16     |   -+-
    |  16      | 16     |    |  16   |  32      |  32     |    |
    |  32      | 32     |    |  ...  |  ...     |  ...    |    | Linear Stage
    |  ...     | ...    |    | n /16 |  n / 8   |  n / 8  |    | ( Stages 16, 32, ..., n/16)
    |  n / 8   | n / 8  |    +-------+----------+---------+   -+-
    +----------+--------+    | n / 8 |  n / 4   |  4      |    |
    |  n / 4   | 4      |    |   4   |  n / 2   |  2      |    | Finale
    |  n / 2   | 2      |    |   2   |  no mask |  no msk |    | ( Stages n/8, 4, 2 )
    +----------+---------    +-------+----------+---------+   -+-

    ( The stage is denoted by the mapped comparator width of the starting comparator. )

    We always have n >= 64. When n=64, the finale overlaps the prelude and needs a special case.

    To apply the linear, stage we split each for loop into two parts, one does the non-masked
    and the other does the masked.
  */

  int32_t n = 1 << m;
  int32x8 mask_4s = _mm256_set_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
  int32x8 mask_2s = _mm256_set_epi32(-1, -1, 0, 0, -1, -1, 0, 0);
  int32x8 swap_masks_4s_to_mask_2s = mask_4s ^ mask_2s;

  if ( n == 64 ) {

    int32x8 x0 = int32x8_load(&x[0]);
    int32x8 x1 = int32x8_load(&x[8]);
    int32x8 x2 = int32x8_load(&x[16]);
    int32x8 x3 = int32x8_load(&x[24]);
    int32x8 x4 = int32x8_load(&x[32]);
    int32x8 x5 = int32x8_load(&x[40]);
    int32x8 x6 = int32x8_load(&x[48]);
    int32x8 x7 = int32x8_load(&x[56]);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x3, x1);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x7, x5);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    x0 ^= mask_4s;
    x1 ^= mask_4s;
    x2 ^= mask_4s;
    x3 ^= mask_4s;
    x4 ^= mask_4s;
    x5 ^= mask_4s;
    x6 ^= mask_4s;
    x7 ^= mask_4s;

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);


    x0 ^= mask_4s; 
    x1 ^= mask_4s; 
    x2 ^= mask_4s; 
    x3 ^= mask_4s; 
    x4 ^= mask_4s; 
    x5 ^= mask_4s; 
    x6 ^= mask_4s; 
    x7 ^= mask_4s; 

    x0 ^= mask_2s; /* A01234567 */
    x1 ^= mask_2s; /* B01234567 */
    x2 ^= mask_2s; /* C01234567 */
    x3 ^= mask_2s; /* D01234567 */
    x4 ^= mask_2s; /* E01234567 */
    x5 ^= mask_2s; /* F01234567 */
    x6 ^= mask_2s; /* G01234567 */
    x7 ^= mask_2s; /* H01234567 */

    int32x8 a0 = _mm256_permute2x128_si256(x0,x1,0x20); /* A0123B0123 */
    int32x8 a1 = _mm256_permute2x128_si256(x0,x1,0x31); /* A4567B4567 */
    int32x8 a2 = _mm256_permute2x128_si256(x2,x3,0x20); /* C0123D0123 */
    int32x8 a3 = _mm256_permute2x128_si256(x2,x3,0x31); /* C4567D4567 */  
    int32x8 a4 = _mm256_permute2x128_si256(x4,x5,0x20); /* E0123F0123 */
    int32x8 a5 = _mm256_permute2x128_si256(x4,x5,0x31); /* E4567F4567 */
    int32x8 a6 = _mm256_permute2x128_si256(x6,x7,0x20); /* G0123H0123 */
    int32x8 a7 = _mm256_permute2x128_si256(x6,x7,0x31); /* G4567H4567 */

    int32x8_MINMAX(a0, a1);
    int32x8_MINMAX(a2, a3);
    int32x8_MINMAX(a4, a5);
    int32x8_MINMAX(a6, a7);

    x0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A01234567 */
    x1 = _mm256_permute2x128_si256(a0,a1,0x31); /* B01234567 */
    x2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C01234567 */
    x3 = _mm256_permute2x128_si256(a2,a3,0x31); /* D01234567 */  
    x4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E01234567 */
    x5 = _mm256_permute2x128_si256(a4,a5,0x31); /* F01234567 */
    x6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G01234567 */
    x7 = _mm256_permute2x128_si256(a6,a7,0x31); /* H01234567 */

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    x0 ^= mask_2s; /* A01234567 */
    x1 ^= mask_2s; /* B01234567 */
    x2 ^= mask_2s; /* C01234567 */
    x3 ^= mask_2s; /* D01234567 */
    x4 ^= mask_2s; /* E01234567 */
    x5 ^= mask_2s; /* F01234567 */
    x6 ^= mask_2s; /* G01234567 */
    x7 ^= mask_2s; /* H01234567 */

    a0 = _mm256_unpacklo_epi64(x0, x1); /* A01B01A45B45 */
    a1 = _mm256_unpackhi_epi64(x0, x1); /* A23B23A67B67 */
    a2 = _mm256_unpacklo_epi64(x2, x3); /* C01D01C45D45 */
    a3 = _mm256_unpackhi_epi64(x2, x3); /* C23D23C67D67 */
    a4 = _mm256_unpacklo_epi64(x4, x5); /* E01F01E45F45 */
    a5 = _mm256_unpackhi_epi64(x4, x5); /* E23F23E67F67 */
    a6 = _mm256_unpacklo_epi64(x6, x7); /* G01H01G45H45 */
    a7 = _mm256_unpackhi_epi64(x6, x7); /* G23H23G67H67 */

    int32x8_MINMAX(a0, a1);
    int32x8_MINMAX(a2, a3);
    int32x8_MINMAX(a4, a5);
    int32x8_MINMAX(a6, a7);

    x0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A01B01A23B23 */
    x1 = _mm256_permute2x128_si256(a0,a1,0x31); /* A45B45A67B67 */
    x2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C01D01C23D23 */
    x3 = _mm256_permute2x128_si256(a2,a3,0x31); /* C45D45C67D67 */  
    x4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E01F01E23F23 */
    x5 = _mm256_permute2x128_si256(a4,a5,0x31); /* E45F45E67F67 */
    x6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G01H01G23H23 */
    x7 = _mm256_permute2x128_si256(a6,a7,0x31); /* G45H45G67H67 */

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    a0 = _mm256_permute2x128_si256(x0,x1,0x20); /* A01B01A45B45 */
    a1 = _mm256_permute2x128_si256(x0,x1,0x31); /* A23B23A67B67 */
    a2 = _mm256_permute2x128_si256(x2,x3,0x20); /* C01D01C45D45 */
    a3 = _mm256_permute2x128_si256(x2,x3,0x31); /* C23D23C67D67 */  
    a4 = _mm256_permute2x128_si256(x4,x5,0x20); /* E01F01E45F45 */
    a5 = _mm256_permute2x128_si256(x4,x5,0x31); /* E23F23E67F67 */
    a6 = _mm256_permute2x128_si256(x6,x7,0x20); /* G01H01G45H45 */
    a7 = _mm256_permute2x128_si256(x6,x7,0x31); /* G23H23G67H67 */

    x0 = _mm256_unpacklo_epi64(a0, a1); /* A01234567 */
    x1 = _mm256_unpackhi_epi64(a0, a1); /* B01234567 */
    x2 = _mm256_unpacklo_epi64(a2, a3); /* C01234567 */
    x3 = _mm256_unpackhi_epi64(a2, a3); /* D01234567 */
    x4 = _mm256_unpacklo_epi64(a4, a5); /* E01234567 */
    x5 = _mm256_unpackhi_epi64(a4, a5); /* F01234567 */
    x6 = _mm256_unpacklo_epi64(a6, a7); /* G01234567 */
    x7 = _mm256_unpackhi_epi64(a6, a7); /* H01234567 */

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);


    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);
    
    a0 = _mm256_unpacklo_epi64(x0, x4); /* A01E01A45E45 */
    a1 = _mm256_unpackhi_epi64(x0, x4); /* A23E23A67E67 */
    a2 = _mm256_unpacklo_epi64(x1, x5); /* B01F01B45F45 */
    a3 = _mm256_unpackhi_epi64(x1, x5); /* B23F23B67F67 */
    a4 = _mm256_unpacklo_epi64(x2, x6); /* C01G01C45G45 */
    a5 = _mm256_unpackhi_epi64(x2, x6); /* C23G23C67G67 */
    a6 = _mm256_unpacklo_epi64(x3, x7); /* D01H01D45H45 */
    a7 = _mm256_unpackhi_epi64(x3, x7); /* D23H23D67H67 */

    x0 = _mm256_permute2x128_si256(a0,a4,0x20); /* A01E01C01G01 */
    x1 = _mm256_permute2x128_si256(a0,a4,0x31); /* A45E45C45G45 */
    x2 = _mm256_permute2x128_si256(a1,a5,0x20); /* A23E23C23G23 */
    x3 = _mm256_permute2x128_si256(a1,a5,0x31); /* A67E67C67G67 */  
    x4 = _mm256_permute2x128_si256(a2,a6,0x20); /* B01F01D01H01 */
    x5 = _mm256_permute2x128_si256(a2,a6,0x31); /* B45F45D45H45 */
    x6 = _mm256_permute2x128_si256(a3,a7,0x20); /* B23F23D23H23 */
    x7 = _mm256_permute2x128_si256(a3,a7,0x31); /* B67F67D67H67 */

    int32x8_store(&x[0],  x0);
    int32x8_store(&x[8],  x4);
    int32x8_store(&x[16], x1);
    int32x8_store(&x[24], x5);
    int32x8_store(&x[32], x2);
    int32x8_store(&x[40], x6);
    int32x8_store(&x[48], x3);
    int32x8_store(&x[56], x7);

    return;
  }
  
  /* Prelude */
  int32_t q = n >> 2;
  for (int32_t i = 0; i < q; i += 32) {
    int32x8 x0 = int32x8_load(&x[i      ]);
    int32x8 x1 = int32x8_load(&x[i    +8]);
    int32x8 x2 = int32x8_load(&x[i+1*q  ]);
    int32x8 x3 = int32x8_load(&x[i+1*q+8]);
    int32x8 x4 = int32x8_load(&x[i+2*q  ]);
    int32x8 x5 = int32x8_load(&x[i+2*q+8]);
    int32x8 x6 = int32x8_load(&x[i+3*q  ]);
    int32x8 x7 = int32x8_load(&x[i+3*q+8]);

    /* Stage n/2. */
    /* Layer of width 2q. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    /* Stage n/4. */
    /* Layer of width q. */
    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x3, x1);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x7, x5);

    /* Layer of width 2q. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    /* Stage 8. */
    /* Layer of width 8. */
    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    /* Layer of width q. */
    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    /* Layer of width 2q. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    int32x8_store(&x[i      ], x0);
    int32x8_store(&x[i    +8], x1);
    int32x8_store(&x[i+1*q  ], x2);
    int32x8_store(&x[i+1*q+8], x3);
    int32x8_store(&x[i+2*q  ], x4);
    int32x8_store(&x[i+2*q+8], x5);
    int32x8_store(&x[i+3*q  ], x6);
    int32x8_store(&x[i+3*q+8], x7);

    x0 = int32x8_load(&x[i+16]);
    x1 = int32x8_load(&x[i+24]);
    x2 = int32x8_load(&x[i+1*q+16]);
    x3 = int32x8_load(&x[i+1*q+24]);
    x4 = int32x8_load(&x[i+2*q+16]);
    x5 = int32x8_load(&x[i+2*q+24]);
    x6 = int32x8_load(&x[i+3*q+16]);
    x7 = int32x8_load(&x[i+3*q+24]);

    /* Stage n/2. */
    /* First layer of width 2q. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    /* Stage n/4. */
    /* Layer of width q. */
    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x3, x1);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x7, x5);

    /* Layer of width 2q. */
    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    /* Stage 8. */
    /* Layer of width 8. */
    int32x8_MINMAX(x1, x0);
    int32x8_MINMAX(x3, x2);
    int32x8_MINMAX(x5, x4);
    int32x8_MINMAX(x7, x6);

    /* Layer of width q. */
    int32x8_MINMAX(x2, x0);
    int32x8_MINMAX(x3, x1);
    int32x8_MINMAX(x6, x4);
    int32x8_MINMAX(x7, x5);

    /* Layer of width 2q. */
    int32x8_MINMAX(x4, x0);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    int32x8_store(&x[i+16    ], x0);
    int32x8_store(&x[i+24    ], x1);
    int32x8_store(&x[i+1*q+16], x2);
    int32x8_store(&x[i+1*q+24], x3);
    int32x8_store(&x[i+2*q+16], x4);
    int32x8_store(&x[i+2*q+24], x5);
    int32x8_store(&x[i+3*q+16], x6);
    int32x8_store(&x[i+3*q+24], x7);
  }


  /* Linear stage */
  for (int s = 16; s <= (n>>4); s<<=1) {
    greedy_3_layers_large_mask_w1(x, m, s, 2*s);
    final_3_layers_large_mask_w1(x, m, 2*s);
  }

  /* Finale. */
  /* Stage n / 8. */
  for (int i = 0; i < n; i += 64) {
    int32x8 x0 = int32x8_load(&x[i   ]);
    int32x8 x1 = int32x8_load(&x[i+8 ]);
    int32x8 x2 = int32x8_load(&x[i+16]);
    int32x8 x3 = int32x8_load(&x[i+24]);
    int32x8 x4 = int32x8_load(&x[i+32]);
    int32x8 x5 = int32x8_load(&x[i+40]);
    int32x8 x6 = int32x8_load(&x[i+48]);
    int32x8 x7 = int32x8_load(&x[i+56]);

    x0 ^= mask_4s;
    x1 ^= mask_4s;
    x2 ^= mask_4s;
    x3 ^= mask_4s;
    x4 ^= mask_4s;
    x5 ^= mask_4s;
    x6 ^= mask_4s;
    x7 ^= mask_4s;

    int32x8_store(&x[i   ], x0);
    int32x8_store(&x[i+8 ], x1);
    int32x8_store(&x[i+16], x2);
    int32x8_store(&x[i+24], x3);
    int32x8_store(&x[i+32], x4);
    int32x8_store(&x[i+40], x5);
    int32x8_store(&x[i+48], x6);
    int32x8_store(&x[i+56], x7);
  }

  greedy_3_layers_small_mask_w1(x, m, n>>3);
  final_3_layers_small_mask_w1(x, m, swap_masks_4s_to_mask_2s);

  /* Stage n / 4. */
    q = n >> 3;
  for (int i = 0; i < q; i += 8) {
    /* Comparisons of size 4, n/8. */
    int32x8 x0 = int32x8_load(&x[i+0*q]); /* A01234567 */ 
    int32x8 x1 = int32x8_load(&x[i+1*q]); /* B01234567 */
    int32x8 x2 = int32x8_load(&x[i+2*q]); /* C01234567 */
    int32x8 x3 = int32x8_load(&x[i+3*q]); /* D01234567 */
    int32x8 x4 = int32x8_load(&x[i+4*q]); /* E01234567 */
    int32x8 x5 = int32x8_load(&x[i+5*q]); /* F01234567 */
    int32x8 x6 = int32x8_load(&x[i+6*q]); /* G01234567 */
    int32x8 x7 = int32x8_load(&x[i+7*q]); /* H01234567 */

    int32x8 a0 = _mm256_permute2x128_si256(x0,x1,0x20); /* A0123B0123 */
    int32x8 a1 = _mm256_permute2x128_si256(x0,x1,0x31); /* A4567B4567 */
    int32x8 a2 = _mm256_permute2x128_si256(x2,x3,0x20); /* C0123D0123 */
    int32x8 a3 = _mm256_permute2x128_si256(x2,x3,0x31); /* C4567D4567 */
    int32x8 a4 = _mm256_permute2x128_si256(x4,x5,0x20); /* E0123F0123 */
    int32x8 a5 = _mm256_permute2x128_si256(x4,x5,0x31); /* E4567F4567 */
    int32x8 a6 = _mm256_permute2x128_si256(x6,x7,0x20); /* G0123H0123 */
    int32x8 a7 = _mm256_permute2x128_si256(x6,x7,0x31); /* G4567H4567 */

    int32x8_MINMAX(a0, a1);
    int32x8_MINMAX(a2, a3);
    int32x8_MINMAX(a4, a5);
    int32x8_MINMAX(a6, a7);

    x0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A01234567 */
    x1 = _mm256_permute2x128_si256(a0,a1,0x31); /* B01234567 */
    x2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C01234567 */
    x3 = _mm256_permute2x128_si256(a2,a3,0x31); /* D01234567 */
    x4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E01234567 */
    x5 = _mm256_permute2x128_si256(a4,a5,0x31); /* F01234567 */
    x6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G01234567 */
    x7 = _mm256_permute2x128_si256(a6,a7,0x31); /* H01234567 */

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_store(&x[i+0*q], x0);  
    int32x8_store(&x[i+1*q], x1); 
    int32x8_store(&x[i+2*q], x2); 
    int32x8_store(&x[i+3*q], x3); 
    int32x8_store(&x[i+4*q], x4); 
    int32x8_store(&x[i+5*q], x5); 
    int32x8_store(&x[i+6*q], x6); 
    int32x8_store(&x[i+7*q], x7); 
  }

  greedy_3_layers_small_mask_w1(x, m, n>>4);
  final_3_layers_small_mask_w1(x, m, mask_2s);

  /* Stage n / 2. */
  q = n >> 3;
  for (int i = 0; i < q; i += 8) {
    /* Comparisons of size 2, 4, n/8. */
    int32x8 x0 = int32x8_load(&x[i+0*q]); /* A01234567 */ 
    int32x8 x1 = int32x8_load(&x[i+1*q]); /* B01234567 */
    int32x8 x2 = int32x8_load(&x[i+2*q]); /* C01234567 */
    int32x8 x3 = int32x8_load(&x[i+3*q]); /* D01234567 */
    int32x8 x4 = int32x8_load(&x[i+4*q]); /* E01234567 */
    int32x8 x5 = int32x8_load(&x[i+5*q]); /* F01234567 */
    int32x8 x6 = int32x8_load(&x[i+6*q]); /* G01234567 */
    int32x8 x7 = int32x8_load(&x[i+7*q]); /* H01234567 */

    int32x8 a0 = _mm256_unpacklo_epi64(x0, x1); /* A01B01A45B45 */
    int32x8 a1 = _mm256_unpackhi_epi64(x0, x1); /* A23B23A67B67 */
    int32x8 a2 = _mm256_unpacklo_epi64(x2, x3); /* C01D01C45D45 */
    int32x8 a3 = _mm256_unpackhi_epi64(x2, x3); /* C23D23C67D67 */
    int32x8 a4 = _mm256_unpacklo_epi64(x4, x5); /* E01F01E45F45 */
    int32x8 a5 = _mm256_unpackhi_epi64(x4, x5); /* E23F23E67F67 */
    int32x8 a6 = _mm256_unpacklo_epi64(x6, x7); /* G01H01G45H45 */
    int32x8 a7 = _mm256_unpackhi_epi64(x6, x7); /* G23H23G67H67 */

    int32x8_MINMAX(a0, a1);
    int32x8_MINMAX(a2, a3);
    int32x8_MINMAX(a4, a5);
    int32x8_MINMAX(a6, a7);

    x0 = _mm256_unpacklo_epi64(a0, a1); /* A01234567 */
    x1 = _mm256_unpackhi_epi64(a0, a1); /* B01234567 */
    x2 = _mm256_unpacklo_epi64(a2, a3); /* C01234567 */
    x3 = _mm256_unpackhi_epi64(a2, a3); /* D01234567 */
    x4 = _mm256_unpacklo_epi64(a4, a5); /* E01234567 */
    x5 = _mm256_unpackhi_epi64(a4, a5); /* F01234567 */
    x6 = _mm256_unpacklo_epi64(a6, a7); /* G01234567 */
    x7 = _mm256_unpackhi_epi64(a6, a7); /* H01234567 */

    a0 = _mm256_permute2x128_si256(x0,x1,0x20); /* A0123B0123 */
    a1 = _mm256_permute2x128_si256(x0,x1,0x31); /* A4567B4567 */
    a2 = _mm256_permute2x128_si256(x2,x3,0x20); /* C0123D0123 */
    a3 = _mm256_permute2x128_si256(x2,x3,0x31); /* C4567D4567 */
    a4 = _mm256_permute2x128_si256(x4,x5,0x20); /* E0123F0123 */
    a5 = _mm256_permute2x128_si256(x4,x5,0x31); /* E4567F4567 */
    a6 = _mm256_permute2x128_si256(x6,x7,0x20); /* G0123H0123 */
    a7 = _mm256_permute2x128_si256(x6,x7,0x31); /* G4567H4567 */

    int32x8_MINMAX(a0, a1);
    int32x8_MINMAX(a2, a3);
    int32x8_MINMAX(a4, a5);
    int32x8_MINMAX(a6, a7);

    x0 = _mm256_permute2x128_si256(a0,a1,0x20); /* A01234567 */
    x1 = _mm256_permute2x128_si256(a0,a1,0x31); /* B01234567 */
    x2 = _mm256_permute2x128_si256(a2,a3,0x20); /* C01234567 */
    x3 = _mm256_permute2x128_si256(a2,a3,0x31); /* D01234567 */
    x4 = _mm256_permute2x128_si256(a4,a5,0x20); /* E01234567 */
    x5 = _mm256_permute2x128_si256(a4,a5,0x31); /* F01234567 */
    x6 = _mm256_permute2x128_si256(a6,a7,0x20); /* G01234567 */
    x7 = _mm256_permute2x128_si256(a6,a7,0x31); /* H01234567 */

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_store(&x[i+0*q], x0);  
    int32x8_store(&x[i+1*q], x1); 
    int32x8_store(&x[i+2*q], x2); 
    int32x8_store(&x[i+3*q], x3); 
    int32x8_store(&x[i+4*q], x4); 
    int32x8_store(&x[i+5*q], x5); 
    int32x8_store(&x[i+6*q], x6); 
    int32x8_store(&x[i+7*q], x7); 
  }

  int32x8 mask_0s = mask_2s ^ mask_2s;
  greedy_3_layers_small_mask_w1(x, m, n>>4);
  final_3_layers_small_mask_w1(x, m, mask_0s);

  /* Apply perm and store. */
  q = n >> 2;
  for (int i = 0;i < q;i += 8) {
    int32x8 a0 = int32x8_load(&x[i    ]); /* A01234567 */
    int32x8 a1 = int32x8_load(&x[i+1*q]); /* B01234567 */ 
    int32x8 a2 = int32x8_load(&x[i+2*q]); /* C01234567 */
    int32x8 a3 = int32x8_load(&x[i+3*q]); /* D01234567 */

    int32x8 b0 = _mm256_unpacklo_epi64(a0,a2); /* A01C01A45C45 */
    int32x8 b1 = _mm256_unpackhi_epi64(a0,a2); /* A23C23A67C67 */
    int32x8 b2 = _mm256_unpacklo_epi64(a1,a3); /* B01D01B45D45 */
    int32x8 b3 = _mm256_unpackhi_epi64(a1,a3); /* B23D23B67D67 */

    int32x8 c0 = _mm256_permute2x128_si256(b0,b2,0x20); /* A01C01B01D01 */
    int32x8 c1 = _mm256_permute2x128_si256(b1,b3,0x20); /* A23C23B23D23 */
    int32x8 c2 = _mm256_permute2x128_si256(b0,b2,0x31); /* A45C45B45D45 */
    int32x8 c3 = _mm256_permute2x128_si256(b1,b3,0x31); /* A67C67B67D67 */

    int32x8_store(&x[i],c0);      /* A01C01B01D01 */
    int32x8_store(&x[i+q],c2);    /* A45C45B45D45 */
    int32x8_store(&x[i+2*q],c1);  /* A23C23B23D23 */
    int32x8_store(&x[i+3*q],c3);  /* A67C67B67D67 */

  }
}


