#include "../../../utilities/utilities.h"

static void iperm_w2(int32_t *x, int32_t m);
/* 
    Scroll down to function iperm_w1 for the description. 
    In the 'large mask' functions, we complement lanes to avoid xoring in the masks.
*/
static inline void greedy_3_layers_small_mask_w2(int32_t *x, int32_t m, int32_t start) {
  /* Applies the decreasing chain of sorts from start to 32 inclusive. 
     REQUIRES: n >= 64. We greedy apply three layers at a time to reduce
     loads and stores. */
  int32_t n = 1 << m;
  int32_t u = start;
  int32_t l;
  while (u >> 2 >= 32) {
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

  if ( u == 32 ) {
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
  else if ( u == 64) {
    for (int b = 0; b < n; b += 128) {
      int32x8 x0 = int32x8_load(&x[b    ]);
      int32x8 x1 = int32x8_load(&x[b+8  ]);
      int32x8 x2 = int32x8_load(&x[b+32 ]);
      int32x8 x3 = int32x8_load(&x[b+40 ]);
      int32x8 x4 = int32x8_load(&x[b+64 ]);
      int32x8 x5 = int32x8_load(&x[b+72 ]);
      int32x8 x6 = int32x8_load(&x[b+96 ]);
      int32x8 x7 = int32x8_load(&x[b+104]);

      int32x8_MINMAX(x0, x4);
      int32x8_MINMAX(x1, x5);
      int32x8_MINMAX(x2, x6);
      int32x8_MINMAX(x3, x7);

      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7);

      int32x8_store(&x[b    ], x0);
      int32x8_store(&x[b+8  ], x1);
      int32x8_store(&x[b+32 ], x2);
      int32x8_store(&x[b+40 ], x3);
      int32x8_store(&x[b+64 ], x4);
      int32x8_store(&x[b+72 ], x5);
      int32x8_store(&x[b+96 ], x6);
      int32x8_store(&x[b+104], x7);

      x0 = int32x8_load(&x[b+16 ]);
      x1 = int32x8_load(&x[b+24 ]);
      x2 = int32x8_load(&x[b+48 ]);
      x3 = int32x8_load(&x[b+56 ]);
      x4 = int32x8_load(&x[b+80 ]);
      x5 = int32x8_load(&x[b+88 ]);
      x6 = int32x8_load(&x[b+112]);
      x7 = int32x8_load(&x[b+120]);

      int32x8_MINMAX(x0, x4);
      int32x8_MINMAX(x1, x5);
      int32x8_MINMAX(x2, x6);
      int32x8_MINMAX(x3, x7);

      int32x8_MINMAX(x0, x2);
      int32x8_MINMAX(x1, x3);
      int32x8_MINMAX(x4, x6);
      int32x8_MINMAX(x5, x7);

      int32x8_store(&x[b+16 ], x0);
      int32x8_store(&x[b+24 ], x1);
      int32x8_store(&x[b+48 ], x2);
      int32x8_store(&x[b+56 ], x3);
      int32x8_store(&x[b+80 ], x4);
      int32x8_store(&x[b+88 ], x5);
      int32x8_store(&x[b+112], x6);
      int32x8_store(&x[b+120], x7);
    }
  } 
}

static inline void greedy_3_layers_large_mask_w2(int32_t *x, int32_t m, int32_t start, int32_t mask) {
  int32_t n = 1 << m;
  int32_t u = start;
  int32_t l;

  while ( u >> 2 >= 32) {
    /* This is reached when u >= 128 ==> mask >= 256. */
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

  if ( u == 32 ) {
    /* Smallest mask size is 2*u >= 64. */
    for (int b = 0; b < n; b+= 2 * mask) {
      for (int i = 0; i < mask; i += 64) {
        int32x8 x0 = int32x8_load(&x[b+i   ]);
        int32x8 x1 = int32x8_load(&x[b+i+8 ]);
        int32x8 x2 = int32x8_load(&x[b+i+16]);
        int32x8 x3 = int32x8_load(&x[b+i+24]);
        int32x8 x4 = int32x8_load(&x[b+i+32]);
        int32x8 x5 = int32x8_load(&x[b+i+40]);
        int32x8 x6 = int32x8_load(&x[b+i+48]);
        int32x8 x7 = int32x8_load(&x[b+i+56]);

        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_store(&x[b+i   ], x0);
        int32x8_store(&x[b+i+8 ], x1);
        int32x8_store(&x[b+i+16], x2);
        int32x8_store(&x[b+i+24], x3);
        int32x8_store(&x[b+i+32], x4);
        int32x8_store(&x[b+i+40], x5);
        int32x8_store(&x[b+i+48], x6);
        int32x8_store(&x[b+i+56], x7);

        x0 = int32x8_load(&x[mask+b+i   ]);
        x1 = int32x8_load(&x[mask+b+i+8 ]);
        x2 = int32x8_load(&x[mask+b+i+16]);
        x3 = int32x8_load(&x[mask+b+i+24]);
        x4 = int32x8_load(&x[mask+b+i+32]);
        x5 = int32x8_load(&x[mask+b+i+40]);
        x6 = int32x8_load(&x[mask+b+i+48]);
        x7 = int32x8_load(&x[mask+b+i+56]);

        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);

        int32x8_store(&x[mask+b+i   ], x0);
        int32x8_store(&x[mask+b+i+8 ], x1);
        int32x8_store(&x[mask+b+i+16], x2);
        int32x8_store(&x[mask+b+i+24], x3);
        int32x8_store(&x[mask+b+i+32], x4);
        int32x8_store(&x[mask+b+i+40], x5);
        int32x8_store(&x[mask+b+i+48], x6);
        int32x8_store(&x[mask+b+i+56], x7);
      }
    }
  } else if ( u == 64) 
  {
    /* Smallest mask size is 2*u >= 128. */
    for (int b = 0; b < n; b+= 2 * mask) {
      for (int i = 0; i < mask; i+=128) {

        /* Two unmasked load/stores. */
        int32x8 x0 = int32x8_load(&x[b+i    ]);
        int32x8 x1 = int32x8_load(&x[b+i+8  ]);
        int32x8 x2 = int32x8_load(&x[b+i+32 ]);
        int32x8 x3 = int32x8_load(&x[b+i+40 ]);
        int32x8 x4 = int32x8_load(&x[b+i+64 ]);
        int32x8 x5 = int32x8_load(&x[b+i+72 ]);
        int32x8 x6 = int32x8_load(&x[b+i+96 ]);
        int32x8 x7 = int32x8_load(&x[b+i+104]);

        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);

        int32x8_store(&x[b+i    ], x0);
        int32x8_store(&x[b+i+8  ], x1);
        int32x8_store(&x[b+i+32 ], x2);
        int32x8_store(&x[b+i+40 ], x3);
        int32x8_store(&x[b+i+64 ], x4);
        int32x8_store(&x[b+i+72 ], x5);
        int32x8_store(&x[b+i+96 ], x6);
        int32x8_store(&x[b+i+104], x7);

        x0 = int32x8_load(&x[b+i+16 ]);
        x1 = int32x8_load(&x[b+i+24 ]);
        x2 = int32x8_load(&x[b+i+48 ]);
        x3 = int32x8_load(&x[b+i+56 ]);
        x4 = int32x8_load(&x[b+i+80 ]);
        x5 = int32x8_load(&x[b+i+88 ]);
        x6 = int32x8_load(&x[b+i+112]);
        x7 = int32x8_load(&x[b+i+120]);

        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);

        int32x8_store(&x[b+i+16 ], x0);
        int32x8_store(&x[b+i+24 ], x1);
        int32x8_store(&x[b+i+48 ], x2);
        int32x8_store(&x[b+i+56 ], x3);
        int32x8_store(&x[b+i+80 ], x4);
        int32x8_store(&x[b+i+88 ], x5);
        int32x8_store(&x[b+i+112], x6);
        int32x8_store(&x[b+i+120], x7);

        /* Two masked load/stores. */
        x0 = int32x8_load(&x[mask+b+i    ]);
        x1 = int32x8_load(&x[mask+b+i+8  ]);
        x2 = int32x8_load(&x[mask+b+i+32 ]);
        x3 = int32x8_load(&x[mask+b+i+40 ]);
        x4 = int32x8_load(&x[mask+b+i+64 ]);
        x5 = int32x8_load(&x[mask+b+i+72 ]);
        x6 = int32x8_load(&x[mask+b+i+96 ]);
        x7 = int32x8_load(&x[mask+b+i+104]);

        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);

        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_store(&x[mask+b+i    ], x0);
        int32x8_store(&x[mask+b+i+8  ], x1);
        int32x8_store(&x[mask+b+i+32 ], x2);
        int32x8_store(&x[mask+b+i+40 ], x3);
        int32x8_store(&x[mask+b+i+64 ], x4);
        int32x8_store(&x[mask+b+i+72 ], x5);
        int32x8_store(&x[mask+b+i+96 ], x6);
        int32x8_store(&x[mask+b+i+104], x7);

        x0 = int32x8_load(&x[mask+b+i+16 ]);
        x1 = int32x8_load(&x[mask+b+i+24 ]);
        x2 = int32x8_load(&x[mask+b+i+48 ]);
        x3 = int32x8_load(&x[mask+b+i+56 ]);
        x4 = int32x8_load(&x[mask+b+i+80 ]);
        x5 = int32x8_load(&x[mask+b+i+88 ]);
        x6 = int32x8_load(&x[mask+b+i+112]);
        x7 = int32x8_load(&x[mask+b+i+120]);

        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);

        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_store(&x[mask+b+i+16 ], x0);
        int32x8_store(&x[mask+b+i+24 ], x1);
        int32x8_store(&x[mask+b+i+48 ], x2);
        int32x8_store(&x[mask+b+i+56 ], x3);
        int32x8_store(&x[mask+b+i+80 ], x4);
        int32x8_store(&x[mask+b+i+88 ], x5);
        int32x8_store(&x[mask+b+i+112], x6);
        int32x8_store(&x[mask+b+i+120], x7);
      }
    }
  }
  
}


static void iperm_w2(int32_t *x, int32_t m) {
  /* A description of the mapping applied to the comparator network
     is as follows:
    
    +-------------------+    +----------------------------+
    | comparator widths |    |   mask alternating period  |
    +----------+--------+    +----------------------------+
    | original | mapped |    | stage | original | mappped |
    +----------+--------+    +-------+----------+---------+  -+-
    |  4       | n / 2  |    | n / 2 | 8        | 8       |   | Prelude
    |  8       | 8      |    | 8     | 16       | 16      |   | ( Stages n/2, 8, 16 )
    | 16       | 16     |    | 16    | 32       | 32      |  -+-
    | 32       | 32     |    | 32    | 64       | 64      |   |
    | ...      | ...    |    | ...   | ...      | ...     |   | Linear Stage
    | n / 8    | n / 8  |    | n / 8 | n / 4    | n / 4   |   | ( Stages 32 to n/8 inclusive)
    +----------+--------+    +-------+----------+---------+  -+-
    | n / 4    | n / 4  |    | n / 4 | n / 2    | 4       |   | Finale
    | n / 2    | 4      |    | 4     | no mask  | no mask |   | ( Stages n/4, 4 )
    +----------+--------+    +-------+----------+---------+  -+-

    ( The stage is denoted by the mapped comparator width of the starting comparator. )

    In the case n = 64, the prelude overlaps into the finale. We assign a special case.
    */
  
  
  int32_t n = 1 << m;
  int32x8 mask_4s = _mm256_set_epi32(-1, -1, -1, -1, 0, 0, 0, 0);

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
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x3, x2);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x7, x6);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    x0 ^= mask_4s;
    x1 ^= mask_4s;
    x2 ^= mask_4s;
    x3 ^= mask_4s;
    x4 ^= mask_4s;
    x5 ^= mask_4s;
    x6 ^= mask_4s;
    x7 ^= mask_4s;

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

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

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    a0 = _mm256_permute2x128_si256(x0,x4,0x20); /* A0123E0123 */
    a1 = _mm256_permute2x128_si256(x0,x4,0x31); /* A4567E4567 */
    a2 = _mm256_permute2x128_si256(x1,x5,0x20); /* B0123F0123 */
    a3 = _mm256_permute2x128_si256(x1,x5,0x31); /* B4567F4567 */
    a4 = _mm256_permute2x128_si256(x2,x6,0x20); /* C0123G0123 */
    a5 = _mm256_permute2x128_si256(x2,x6,0x31); /* C4567G4567 */
    a6 = _mm256_permute2x128_si256(x3,x7,0x20); /* D0123H0123 */
    a7 = _mm256_permute2x128_si256(x3,x7,0x31); /* D4567H4567 */


    int32x8_store(&x[0] , a0);
    int32x8_store(&x[8] , a2);
    int32x8_store(&x[16], a4);
    int32x8_store(&x[24], a6);
    int32x8_store(&x[32], a1);
    int32x8_store(&x[40], a3);
    int32x8_store(&x[48], a5);
    int32x8_store(&x[56], a7);
    return;
  }

  /* Prelude */
  int32_t q = n >> 1;
  for (int i = 0; i < q; i += 64) {
    int32x8 x0 = int32x8_load(&x[i+0   ]);
    int32x8 x1 = int32x8_load(&x[i+8   ]);
    int32x8 x2 = int32x8_load(&x[i+16  ]);
    int32x8 x3 = int32x8_load(&x[i+24  ]);
    int32x8 x4 = int32x8_load(&x[i+q+0 ]);
    int32x8 x5 = int32x8_load(&x[i+q+8 ]);
    int32x8 x6 = int32x8_load(&x[i+q+16]);
    int32x8 x7 = int32x8_load(&x[i+q+24]);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x3, x2);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x7, x6);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    int32x8_store(&x[i+0   ], x0);
    int32x8_store(&x[i+8   ], x1);
    int32x8_store(&x[i+16  ], x2);
    int32x8_store(&x[i+24  ], x3);
    int32x8_store(&x[i+q+0 ], x4);
    int32x8_store(&x[i+q+8 ], x5);
    int32x8_store(&x[i+q+16], x6);
    int32x8_store(&x[i+q+24], x7);

    x0 = int32x8_load(&x[i+0   +32]);
    x1 = int32x8_load(&x[i+8   +32]);
    x2 = int32x8_load(&x[i+16  +32]);
    x3 = int32x8_load(&x[i+24  +32]);
    x4 = int32x8_load(&x[i+q+0 +32]);
    x5 = int32x8_load(&x[i+q+8 +32]);
    x6 = int32x8_load(&x[i+q+16+32]);
    x7 = int32x8_load(&x[i+q+24+32]);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x7, x3);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x3, x2);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x7, x6);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
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

    int32x8_MINMAX(x4, x0);
    int32x8_MINMAX(x5, x1);
    int32x8_MINMAX(x6, x2);
    int32x8_MINMAX(x7, x3);

    int32x8_store(&x[i+0   +32], x0);
    int32x8_store(&x[i+8   +32], x1);
    int32x8_store(&x[i+16  +32], x2);
    int32x8_store(&x[i+24  +32], x3);
    int32x8_store(&x[i+q+0 +32], x4);
    int32x8_store(&x[i+q+8 +32], x5);
    int32x8_store(&x[i+q+16+32], x6);
    int32x8_store(&x[i+q+24+32], x7);
  }
  
  /* Linear stage. */
  for (int s = 32; s <= (n>>3); s<<=1) {
    int32_t mask = 2*s;
    greedy_3_layers_large_mask_w2(x, m, s, mask);
    
    /* Final three layers (16, 8, n/2). */
    q = n >> 1;
    for (int b = 0; b < q; b += 2*mask) {
      for (int i = 0; i < mask; i += 32 ) {
        int32x8 x0 = int32x8_load(&x[b+i+0 ]);
        int32x8 x1 = int32x8_load(&x[b+i+8 ]);
        int32x8 x2 = int32x8_load(&x[b+i+16]);
        int32x8 x3 = int32x8_load(&x[b+i+24]);
        int32x8 x4 = int32x8_load(&x[b+i+q+0 ]);
        int32x8 x5 = int32x8_load(&x[b+i+q+8 ]);
        int32x8 x6 = int32x8_load(&x[b+i+q+16]);
        int32x8 x7 = int32x8_load(&x[b+i+q+24]);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);
        int32x8_MINMAX(x4, x6);
        int32x8_MINMAX(x5, x7);

        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);
        int32x8_MINMAX(x4, x5);
        int32x8_MINMAX(x6, x7);

        int32x8_MINMAX(x0, x4);
        int32x8_MINMAX(x1, x5);
        int32x8_MINMAX(x2, x6);
        int32x8_MINMAX(x3, x7);

        int32x8_store(&x[b+i+0 ]  , x0);
        int32x8_store(&x[b+i+8 ]  , x1);
        int32x8_store(&x[b+i+16]  , x2);
        int32x8_store(&x[b+i+24]  , x3);
        int32x8_store(&x[b+i+q+0 ], x4);
        int32x8_store(&x[b+i+q+8 ], x5);
        int32x8_store(&x[b+i+q+16], x6);
        int32x8_store(&x[b+i+q+24], x7);

        x0 = int32x8_load(&x[mask+b+i+0 ]);
        x1 = int32x8_load(&x[mask+b+i+8 ]);
        x2 = int32x8_load(&x[mask+b+i+16]);
        x3 = int32x8_load(&x[mask+b+i+24]);
        x4 = int32x8_load(&x[mask+b+i+q+0 ]);
        x5 = int32x8_load(&x[mask+b+i+q+8 ]);
        x6 = int32x8_load(&x[mask+b+i+q+16]);
        x7 = int32x8_load(&x[mask+b+i+q+24]);

        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);
        int32x8_MINMAX(x6, x4);
        int32x8_MINMAX(x7, x5);

        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x3, x2);
        int32x8_MINMAX(x5, x4);
        int32x8_MINMAX(x7, x6);

        int32x8_MINMAX(x4, x0);
        int32x8_MINMAX(x5, x1);
        int32x8_MINMAX(x6, x2);
        int32x8_MINMAX(x7, x3);

        int32x8_store(&x[mask+b+i+0 ]  , x0);
        int32x8_store(&x[mask+b+i+8 ]  , x1);
        int32x8_store(&x[mask+b+i+16]  , x2);
        int32x8_store(&x[mask+b+i+24]  , x3);
        int32x8_store(&x[mask+b+i+q+0 ], x4);
        int32x8_store(&x[mask+b+i+q+8 ], x5);
        int32x8_store(&x[mask+b+i+q+16], x6);
        int32x8_store(&x[mask+b+i+q+24], x7);
      }
    }
  }

  /* Finale. */
  for (int i = 0; i < n; i += 64) {
    int32x8 x0 = int32x8_load(&x[i+0 ]);
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

    int32x8_store(&x[i+0 ], x0);
    int32x8_store(&x[i+8 ], x1);
    int32x8_store(&x[i+16], x2);
    int32x8_store(&x[i+24], x3);
    int32x8_store(&x[i+32], x4);
    int32x8_store(&x[i+40], x5);
    int32x8_store(&x[i+48], x6);
    int32x8_store(&x[i+56], x7);
  }
  greedy_3_layers_small_mask_w2(x, m, n >> 2);

  q = n >> 1;
  for (int i = 0; i < q; i += 32) {
    /* Final three layers (16, 8, n/2) of stage n/4 AND first layer (4) of stage 4. */
    int32x8 x0 = int32x8_load(&x[i+0   ]);
    int32x8 x1 = int32x8_load(&x[i+8   ]);
    int32x8 x2 = int32x8_load(&x[i+16  ]);
    int32x8 x3 = int32x8_load(&x[i+24  ]);
    int32x8 x4 = int32x8_load(&x[i+q+0 ]);
    int32x8 x5 = int32x8_load(&x[i+q+8 ]);
    int32x8 x6 = int32x8_load(&x[i+q+16]);
    int32x8 x7 = int32x8_load(&x[i+q+24]);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);

    x0 ^= mask_4s; /* A01234567 */
    x1 ^= mask_4s; /* B01234567 */
    x2 ^= mask_4s; /* C01234567 */
    x3 ^= mask_4s; /* D01234567 */
    x4 ^= mask_4s; /* E01234567 */
    x5 ^= mask_4s; /* F01234567 */
    x6 ^= mask_4s; /* G01234567 */
    x7 ^= mask_4s; /* H01234567 */

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

    int32x8_store(&x[i+0   ], x0);
    int32x8_store(&x[i+8   ], x1);
    int32x8_store(&x[i+16  ], x2);
    int32x8_store(&x[i+24  ], x3);
    int32x8_store(&x[i+q+0 ], x4);
    int32x8_store(&x[i+q+8 ], x5);
    int32x8_store(&x[i+q+16], x6);
    int32x8_store(&x[i+q+24], x7);
  }

  /* Layers n/4 to 32 of final stage 4. */
  greedy_3_layers_small_mask_w2(x, m, n >> 2);
  for (int i = 0; i < q; i += 32) {
    /* Final three layers (16, 8, n/2) of stage 4. */
    int32x8 x0 = int32x8_load(&x[i+0   ]);
    int32x8 x1 = int32x8_load(&x[i+8   ]);
    int32x8 x2 = int32x8_load(&x[i+16  ]);
    int32x8 x3 = int32x8_load(&x[i+24  ]);
    int32x8 x4 = int32x8_load(&x[i+q+0 ]);
    int32x8 x5 = int32x8_load(&x[i+q+8 ]);
    int32x8 x6 = int32x8_load(&x[i+q+16]);
    int32x8 x7 = int32x8_load(&x[i+q+24]);

    int32x8_MINMAX(x0, x2);
    int32x8_MINMAX(x1, x3);
    int32x8_MINMAX(x4, x6);
    int32x8_MINMAX(x5, x7);

    int32x8_MINMAX(x0, x1);
    int32x8_MINMAX(x2, x3);
    int32x8_MINMAX(x4, x5);
    int32x8_MINMAX(x6, x7);

    int32x8_MINMAX(x0, x4);
    int32x8_MINMAX(x1, x5);
    int32x8_MINMAX(x2, x6);
    int32x8_MINMAX(x3, x7);
    
    int32x8 a0 = _mm256_permute2x128_si256(x0,x4,0x20); /* A0123E0123 */
    int32x8 a1 = _mm256_permute2x128_si256(x0,x4,0x31); /* A4567E4567 */
    int32x8 a2 = _mm256_permute2x128_si256(x1,x5,0x20); /* B0123F0123 */
    int32x8 a3 = _mm256_permute2x128_si256(x1,x5,0x31); /* B4567F4567 */
    int32x8 a4 = _mm256_permute2x128_si256(x2,x6,0x20); /* C0123G0123 */
    int32x8 a5 = _mm256_permute2x128_si256(x2,x6,0x31); /* C4567G4567 */
    int32x8 a6 = _mm256_permute2x128_si256(x3,x7,0x20); /* D0123H0123 */
    int32x8 a7 = _mm256_permute2x128_si256(x3,x7,0x31); /* D4567H4567 */

    int32x8_store(&x[i+0   ], a0);
    int32x8_store(&x[i+8   ], a2);
    int32x8_store(&x[i+16  ], a4);
    int32x8_store(&x[i+24  ], a6);
    int32x8_store(&x[i+q+0 ], a1);
    int32x8_store(&x[i+q+8 ], a3);
    int32x8_store(&x[i+q+16], a5);
    int32x8_store(&x[i+q+24], a7);
  }
}