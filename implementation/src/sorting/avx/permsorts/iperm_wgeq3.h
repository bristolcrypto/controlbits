#include "../../../utilities/utilities.h"

static void iperm_w_geq_3(int32_t *x, int32_t m, int32_t w);

static inline void merge_block_unmasked(int32_t *x, int32_t start, int32_t end, int32_t stage, int32_t w) {
  int32_t u = stage;
  while ( (u >> 2) >= (1 << w) ) {
    int32_t l = u >> 2;
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i    ]);
        int32x8 x1 = int32x8_load(&x[c+i+1*l]);
        int32x8 x2 = int32x8_load(&x[c+i+2*l]);
        int32x8 x3 = int32x8_load(&x[c+i+3*l]);
        int32x8 x4 = int32x8_load(&x[c+i+4*l]);
        int32x8 x5 = int32x8_load(&x[c+i+5*l]);
        int32x8 x6 = int32x8_load(&x[c+i+6*l]);
        int32x8 x7 = int32x8_load(&x[c+i+7*l]);
        
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

        int32x8_store(&x[c+i    ], x0);
        int32x8_store(&x[c+i+1*l], x1);
        int32x8_store(&x[c+i+2*l], x2);
        int32x8_store(&x[c+i+3*l], x3);
        int32x8_store(&x[c+i+4*l], x4);
        int32x8_store(&x[c+i+5*l], x5);
        int32x8_store(&x[c+i+6*l], x6);
        int32x8_store(&x[c+i+7*l], x7);
      }
    }
    u >>= 3;
  }

  if ( u == (1 << w)) {
    int32_t l = (1 << w);
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i         ]);
        int32x8 x1 = int32x8_load(&x[c+i+l]);
        int32x8_MINMAX(x0, x1);
        int32x8_store(&x[c+i  ], x0);
        int32x8_store(&x[c+i+l], x1);
      }
    }
  } 
  else if ( u == (1 << (w+1)) )
  {
    int32_t l = (1 << w);
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i    ]);
        int32x8 x1 = int32x8_load(&x[c+i+1*l]);
        int32x8 x2 = int32x8_load(&x[c+i+2*l]);
        int32x8 x3 = int32x8_load(&x[c+i+3*l]);

        int32x8_MINMAX(x0, x2);
        int32x8_MINMAX(x1, x3);

        int32x8_MINMAX(x0, x1);
        int32x8_MINMAX(x2, x3);

        int32x8_store(&x[c+i    ], x0);
        int32x8_store(&x[c+i+1*l], x1);
        int32x8_store(&x[c+i+2*l], x2);
        int32x8_store(&x[c+i+3*l], x3);
      }
    }
  }
}

static inline void merge_block_masked(int32_t *x, int32_t start, int32_t end, int32_t stage, int32_t w) {
  int32_t u = stage;
  while ( (u >> 2) >= (1 << w) ) {
    int32_t l = u >> 2;
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i    ]);
        int32x8 x1 = int32x8_load(&x[c+i+1*l]);
        int32x8 x2 = int32x8_load(&x[c+i+2*l]);
        int32x8 x3 = int32x8_load(&x[c+i+3*l]);
        int32x8 x4 = int32x8_load(&x[c+i+4*l]);
        int32x8 x5 = int32x8_load(&x[c+i+5*l]);
        int32x8 x6 = int32x8_load(&x[c+i+6*l]);
        int32x8 x7 = int32x8_load(&x[c+i+7*l]);
        
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

        int32x8_store(&x[c+i    ], x0);
        int32x8_store(&x[c+i+1*l], x1);
        int32x8_store(&x[c+i+2*l], x2);
        int32x8_store(&x[c+i+3*l], x3);
        int32x8_store(&x[c+i+4*l], x4);
        int32x8_store(&x[c+i+5*l], x5);
        int32x8_store(&x[c+i+6*l], x6);
        int32x8_store(&x[c+i+7*l], x7);
      }
    }
    u >>= 3;
  }

  if ( u == (1 << w)) {
    int32_t l = (1 << w);
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i         ]);
        int32x8 x1 = int32x8_load(&x[c+i+l]);
        int32x8_MINMAX(x1, x0);
        int32x8_store(&x[c+i  ], x0);
        int32x8_store(&x[c+i+l], x1);
      }
    }
  } 
  else if ( u == (1 << (w+1)) )
  {
    int32_t l = (1 << w);
    for (int c = start; c < end; c+=2*u) {
      for (int i = 0; i < l; i += 8) {
        int32x8 x0 = int32x8_load(&x[c+i    ]);
        int32x8 x1 = int32x8_load(&x[c+i+1*l]);
        int32x8 x2 = int32x8_load(&x[c+i+2*l]);
        int32x8 x3 = int32x8_load(&x[c+i+3*l]);

        int32x8_MINMAX(x2, x0);
        int32x8_MINMAX(x3, x1);

        int32x8_MINMAX(x1, x0);
        int32x8_MINMAX(x3, x2);

        int32x8_store(&x[c+i    ], x0);
        int32x8_store(&x[c+i+1*l], x1);
        int32x8_store(&x[c+i+2*l], x2);
        int32x8_store(&x[c+i+3*l], x3);
      }
    }
  }
}

static void iperm_w_geq_3(int32_t *x, int32_t m, int32_t w) {

  /* 
   *  For w >= 3, we do not apply any permutation to the comparators. The network
   *  is comprised of interlaced merge blocks. We denote a merge block by the 
   *  comparator width of its first layer. Recall that when m - w = j, the network
   *  'looks like' the bitonic network on 2^j elements.
   *
   *  We again do a depth-first ordering of comparators. For the 'Prelude', we
   *  do stages w, 2w, and 4w. Note that this is the full network when m - w = 3.
   *  Therefore, we break off the cases m - w = 2, 3 and deal with them separately.
   *
   *  Unlike the w = 1, 2 cases, we do not have to do a 'finale' because there is no
   *  permutation applied to the comparators that breaks the structure of the linear
   *  stage. 
   *
   *  On stage w, the mask alternating period is given by 2w. As w>=3, we always have
   *  large mask sizes.
   */
  
  int32_t n = 1 << m;
  int32_t window, block;
  int32_t smallest_jump_size = (1 << w);

  if ( m - w <= 2) {
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
    return;
  }
  else if (m - w <= 3) {
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[                       window]);
      int32x8 x1 = int32x8_load(&x[1*smallest_jump_size + window]);
      int32x8 x2 = int32x8_load(&x[2*smallest_jump_size + window]);
      int32x8 x3 = int32x8_load(&x[3*smallest_jump_size + window]);
      int32x8 x4 = int32x8_load(&x[4*smallest_jump_size + window]);
      int32x8 x5 = int32x8_load(&x[5*smallest_jump_size + window]);
      int32x8 x6 = int32x8_load(&x[6*smallest_jump_size + window]);
      int32x8 x7 = int32x8_load(&x[7*smallest_jump_size + window]);

      /* Stage w. */
      int32x8_MINMAX(x0, x1); 
      int32x8_MINMAX(x3, x2); 
      int32x8_MINMAX(x4, x5);
      int32x8_MINMAX(x7, x6);

      /* Stage 2w. */
      int32x8_MINMAX(x0, x2); 
      int32x8_MINMAX(x1, x3); 
      int32x8_MINMAX(x6, x4);
      int32x8_MINMAX(x7, x5);

      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3); 
      int32x8_MINMAX(x5, x4);
      int32x8_MINMAX(x7, x6);

      /* Stage 4w. */
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

      int32x8_store(&x[                       window], x0);
      int32x8_store(&x[1*smallest_jump_size + window], x1);
      int32x8_store(&x[2*smallest_jump_size + window], x2);
      int32x8_store(&x[3*smallest_jump_size + window], x3);
      int32x8_store(&x[4*smallest_jump_size + window], x4);
      int32x8_store(&x[5*smallest_jump_size + window], x5);
      int32x8_store(&x[6*smallest_jump_size + window], x6);
      int32x8_store(&x[7*smallest_jump_size + window], x7);
    }

    return;
  }




  /* Prelude */ 
  for (block = 0; block < n; block += 16*smallest_jump_size) {
    /* From block+0 to block+8*smallest_jump_size, there is no mask on stage 4w */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[                       block + window]);
      int32x8 x1 = int32x8_load(&x[1*smallest_jump_size + block + window]);
      int32x8 x2 = int32x8_load(&x[2*smallest_jump_size + block + window]);
      int32x8 x3 = int32x8_load(&x[3*smallest_jump_size + block + window]);
      int32x8 x4 = int32x8_load(&x[4*smallest_jump_size + block + window]);
      int32x8 x5 = int32x8_load(&x[5*smallest_jump_size + block + window]);
      int32x8 x6 = int32x8_load(&x[6*smallest_jump_size + block + window]);
      int32x8 x7 = int32x8_load(&x[7*smallest_jump_size + block + window]);

      /* Stage w. */
      int32x8_MINMAX(x0, x1); 
      int32x8_MINMAX(x3, x2); 
      int32x8_MINMAX(x4, x5);
      int32x8_MINMAX(x7, x6);

      /* Stage 2w. */
      int32x8_MINMAX(x0, x2); 
      int32x8_MINMAX(x1, x3); 
      int32x8_MINMAX(x6, x4);
      int32x8_MINMAX(x7, x5);

      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3); 
      int32x8_MINMAX(x5, x4);
      int32x8_MINMAX(x7, x6);

      /* Stage 4w. */
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

      int32x8_store(&x[                       block + window], x0);
      int32x8_store(&x[1*smallest_jump_size + block + window], x1);
      int32x8_store(&x[2*smallest_jump_size + block + window], x2);
      int32x8_store(&x[3*smallest_jump_size + block + window], x3);
      int32x8_store(&x[4*smallest_jump_size + block + window], x4);
      int32x8_store(&x[5*smallest_jump_size + block + window], x5);
      int32x8_store(&x[6*smallest_jump_size + block + window], x6);
      int32x8_store(&x[7*smallest_jump_size + block + window], x7);
    }

    /* From block+8*smallest_jump_size to block+16*smallest_jump_size, 
       there is a mask on stage 4w */
    for (window = 0; window < smallest_jump_size; window+=8) {
      int32x8 x0 = int32x8_load(&x[ 8*smallest_jump_size + block + window]);
      int32x8 x1 = int32x8_load(&x[ 9*smallest_jump_size + block + window]);
      int32x8 x2 = int32x8_load(&x[10*smallest_jump_size + block + window]);
      int32x8 x3 = int32x8_load(&x[11*smallest_jump_size + block + window]);
      int32x8 x4 = int32x8_load(&x[12*smallest_jump_size + block + window]);
      int32x8 x5 = int32x8_load(&x[13*smallest_jump_size + block + window]);
      int32x8 x6 = int32x8_load(&x[14*smallest_jump_size + block + window]);
      int32x8 x7 = int32x8_load(&x[15*smallest_jump_size + block + window]);

      /* Stage w. */
      int32x8_MINMAX(x0, x1); 
      int32x8_MINMAX(x3, x2); 
      int32x8_MINMAX(x4, x5);
      int32x8_MINMAX(x7, x6);

      /* Stage 2w. */
      int32x8_MINMAX(x0, x2); 
      int32x8_MINMAX(x1, x3); 
      int32x8_MINMAX(x6, x4);
      int32x8_MINMAX(x7, x5);

      int32x8_MINMAX(x0, x1);
      int32x8_MINMAX(x2, x3); 
      int32x8_MINMAX(x5, x4);
      int32x8_MINMAX(x7, x6);

      /* Stage 4w. */
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

      int32x8_store(&x[ 8*smallest_jump_size + block + window], x0);
      int32x8_store(&x[ 9*smallest_jump_size + block + window], x1);
      int32x8_store(&x[10*smallest_jump_size + block + window], x2);
      int32x8_store(&x[11*smallest_jump_size + block + window], x3);
      int32x8_store(&x[12*smallest_jump_size + block + window], x4);
      int32x8_store(&x[13*smallest_jump_size + block + window], x5);
      int32x8_store(&x[14*smallest_jump_size + block + window], x6);
      int32x8_store(&x[15*smallest_jump_size + block + window], x7);
    }
  }
  
  /* Linear stage. */
  for (int s = (1 << (w+3)); s <= (n >> 2); s<<=1) {
    int32_t mask = 2*s;
    for (int b = 0; b < n; b += 2*mask) {
      merge_block_unmasked(x,      b,   mask+b, s, w);
      merge_block_masked  (x, mask+b, 2*mask+b, s, w);
    } 
  }
  merge_block_unmasked(x, 0, n, n>>1, w);
}