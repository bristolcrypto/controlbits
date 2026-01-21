#ifndef IBITONIC
#define IBITONIC
#include "ibitonics/base_cases.h"
#include "ibitonics/ibitonic_merge.h"

#if defined(CHOOSE_STANDARD_SORTING)
  #include "permsorts/djbsort.h"
#endif


static void ibitonic(int32_t* x, int32_t m, int32_t w);
static void ibitonic_w_geq_3(int32_t* x, int32_t m, int32_t w);
static void ibitonic_w_leq_2(int32_t* x, int32_t m, int32_t w);



static void ibitonic(int32_t* x, int32_t m, int32_t w){
  #if defined(CHOOSE_STANDARD_SORTING)
  if (m < 6) {
    djbsort(x, 1 << m);
  } else {
    ibitonic_w_leq_2(x, m, 0);
  }

  #elif defined(CHOOSE_INTERLACED_SORTING)

    if (m < 6) {
        printf("Error, ibitonic called as interlaced sorting alg with m < 6.\n");
        exit(1);
    }
    if (w <= 2) {
      ibitonic_w_leq_2(x, m, w);
    } 
    else {
      ibitonic_w_geq_3(x, m, w);
    }
  #endif
}

/* We can be clever here and apply the mask once at the start. And once at the very end. To do... */
void merge_forwards_w_leq_2(int32_t *x, int32_t m, int32_t w) {
  int32_t n = (1 << m);
  int32_t j = m-w;
  int32x8 forward_mask = _mm256_set1_epi32(0);
  
  /* Apply sorts of size 2^{m-1} through to 2^6. */
  if ((j - 6 + w) % 3 == 1) {
    merge_one_stage_forwards(x, m, w, j);
    j--;
  } else if ( (j -6 + w ) % 3 == 2) {
    merge_two_stages_forwards(x, m, w, j);
    j-=2;
  } 

  /* Now j is equal to (6 - w) + multiple of 3*/
  while ( j > 6-w) {
    merge_three_stages_forwards(x, m, w, j);
    j -= 3;
  }

  /* Now j is equal to 6-w. We do all the remaining sorts with our base cases */

  if ( w <= 0 ) {
    for (int chunk = 0; chunk < n; chunk +=64 )
      merge_last_five_stages_w0(x+chunk, forward_mask);
  } else if (w <= 1) {
    for (int chunk = 0; chunk < n; chunk +=64 )
      merge_last_four_stages_w1(x+chunk, forward_mask);
  } else if (w <= 2) {
    for (int chunk = 0; chunk < n; chunk +=64 )
      merge_last_three_stages_w2(x+chunk, forward_mask);
  }
}

/* We can be clever here and apply the mask once at the start. And once at the very end. To do... */
void merge_backwards_w_leq_2(int32_t *x, int32_t m, int32_t w) {
  int32_t n = (1 << m);
  int32_t j = m-w;
  int32x8 reverse_mask = _mm256_set1_epi32(-1);
  
  /* Apply sorts of size 2^{m-1} through to 2^6. */


  if ((j - 6 + w) % 3 == 1) {
    merge_one_stage_backwards(x, m, w, j);
    j--;
  } else if ((j - 6 + w) % 3 == 2){
    merge_two_stages_backwards(x, m, w, j);
    j-=2;
  } 

  /* Now j is equal to (6 - w) + multiple of 3*/
  while ( j > 6-w) {
    merge_three_stages_backwards(x, m, w, j);
    j -= 3;
  }

  /* Now j is equal to 6-w. We do all the remaining sorts with our base cases */

  if ( w <= 0 ) {
    for (int chunk = 0; chunk < n; chunk +=64 )
      merge_last_five_stages_w0(x+chunk, reverse_mask);
  } else if (w <= 1) {
    for (int chunk = 0; chunk < n; chunk +=64 )
      merge_last_four_stages_w1(x+chunk, reverse_mask);
  } else if (w <= 2) {
    for (int chunk = 0; chunk < n; chunk +=64 )  
      merge_last_three_stages_w2(x+chunk, reverse_mask);
  }
}



static void ibitonic_w_leq_2(int32_t* x, int32_t m, int32_t w) {
  int32_t n = 1 << m;
  int32_t chunk_base, chunk_size, depth_of_chunk;
  int32_t depth_of_base_case = m-5;
  int32_t width_of_base_case = 32;
  
  /* Initial sorts. */
  if ( w <= 0 ) {
    for ( int chunk = 0; chunk < n; chunk += 64) 
      chunk_of_64_w0(x+chunk);  
  } else if ( w <= 1 ) {
    for ( int chunk = 0; chunk < n; chunk += 64) 
      chunk_of_64_w1(x+chunk);
  } else {
    for ( int chunk = 0; chunk < n; chunk += 64) 
      chunk_of_64_w2(x+chunk);
  }


  /* Second, perform all the required merges. */
  depth_of_chunk = depth_of_base_case - 1;
  for (chunk_size = 2*width_of_base_case; chunk_size < n; chunk_size <<= 1) {
      for ( chunk_base = 0; chunk_base < n; chunk_base += 2*chunk_size ) {
          merge_forwards_w_leq_2(x + chunk_base, m-depth_of_chunk, w);
          merge_backwards_w_leq_2(x + chunk_base + chunk_size, m-depth_of_chunk, w);
      }
      depth_of_chunk--;
  }
  /* Final merge. */
  merge_forwards_w_leq_2(x, m, w);
}



/* This function is called by cbiterative which always passes in
   the same value of m and increasing values of w where 
   3 <= w <= m-2. The case w = m-1 is covered by the final-bits
   calculation. */
static void ibitonic_w_geq_3(int32_t* x, int32_t m, int32_t w) {
  int32_t n = (1 << m);
  if (m-w <= 1) {
    printf("Error: ibitonic called with m-w <=1");
    exit(1);
  } else if (m-w <= 2)  case_2_with_w_geq_3(x, w);
  else if (m-w <= 3)    case_3_with_w_geq_3(x, w);
  else if (m-w <= 4)    case_4_with_w_geq_3(x, w);
  else if (m-w <= 5) {
      /* Now m-w >= 5. Thus, m >= 8. We decompose this down to the base
          case of m-w = 4 by performing i decompositions. At
          this base stage, we perform 2^i case_4_with_geq_3 
          at offsets of some j * 2^{w+4}.

      We must perform i recursive decompositions
          until m-i-w = 4; */
      //int32_t i = m-w-4;

      int32_t width_of_case4 = (1 << (w+4));
      
      case_4_with_w_geq_3(x, w);
      reversed_case_4_with_w_geq_3(x+width_of_case4, w);
      ibitonic_merge_forwards(x, m, w);

  }
  else if (m-w <= 6) {
      int32_t width_of_case4 = (1 << (w+4));
      
      case_4_with_w_geq_3         (x,                  w);
      reversed_case_4_with_w_geq_3(x+width_of_case4,   w);
      case_4_with_w_geq_3         (x+2*width_of_case4, w);
      reversed_case_4_with_w_geq_3(x+3*width_of_case4, w);
      
      ibitonic_merge_forwards (x,                   m-1, w);
      ibitonic_merge_backwards(x+2*width_of_case4,  m-1, w);

      ibitonic_merge_forwards(x, m, w);

      
  }
  else {
        int32_t block_base, block_size, depth_of_blocks;
        int32_t depth_of_base_case = m-w-4;
        int32_t width_of_base_case = (1 << (w+4));

        /* First perform all of the sorts applied by our base case. */
        for (block_base = 0; block_base < n; block_base += 2*width_of_base_case) {
            case_4_with_w_geq_3(x+block_base, w);
            reversed_case_4_with_w_geq_3(x+block_base+width_of_base_case, w);
        }

        /* Second, perform all the required merges. */
        depth_of_blocks = depth_of_base_case - 1;
        for (block_size = 2*width_of_base_case; block_size < n; block_size <<= 1) {
            for ( block_base = 0; block_base < n; block_base += 2*block_size ) {
                ibitonic_merge_forwards(x + block_base, m-depth_of_blocks, w);
                ibitonic_merge_backwards(x + block_base + block_size, m-depth_of_blocks, w);
            }
            depth_of_blocks--;
        }

        /* Final merge. */
        ibitonic_merge_forwards(x, m, w);
    }
}
#endif