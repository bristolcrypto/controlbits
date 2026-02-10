#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../cryptoint/crypto_int32.h"
#define int32_MINMAX(a,b) crypto_int32_minmax(&(a),&(b))

#if defined(CHOOSE_PORTABLE4)
  #include "portable/portable4.h"
#elif defined(CHOOSE_BITONIC)
  #include "portable/bitonic.h"
#elif defined(CHOOSE_DJBSORT2019)
  #include "avx/permsorts/djbsort20190516.h"
#elif defined(CHOOSE_DJBSORT2026)
  #include "avx/permsorts/djbsort20260127.h"
#elif defined(CHOOSE_IPERMSORT)
  #include "avx/ipermsort.h"
#elif defined(CHOOSE_IBITONIC)
  #include "avx/ibitonic.h"
#endif

/* cbrecursion calls int32_sort, all others call int32_sort_interlaced. */
#if defined(CHOOSE_CBITERATIVE) || defined(CHOOSE_CBITERATIVE_UNROLLED) || defined(CHOOSE_CBITERAVX)
  static void int32_sort_interlaced(int32_t *x, int32_t m, int32_t w) {
    /* We want our iterative variants to be able to call standard sorts & interlaced sorts
     to provide good comparisons. */
    #if defined(CHOOSE_INTERLACED_SORTING)
      #if defined(CHOOSE_PORTABLE4)
        portable4(x, m, w);  
      #elif defined(CHOOSE_IBITONIC)
        ibitonic(x, m, w);
      #elif defined(CHOOSE_IPERMSORT)
        ipermsort(x, m, w);
      #else 
        printf("ERROR: You chose interlaced sorting on an iterative algorithm but int32_sort_interlaced is undefined. \n");
        exit(1);
      #endif
    #elif defined(CHOOSE_STANDARD_SORTING)
      #if defined(CHOOSE_DJBSORT2019)
        djbsort(x, 1 << m);
      #elif defined(CHOOSE_DJBSORT2026)
        djbsort(x, 1 << m);
      #elif defined(CHOOSE_PORTABLE4)
        portable4(x, 1 << m);
      #elif defined(CHOOSE_IBITONIC)
        ibitonic(x, m, 0); /* Force ibitonic with w=0 to give bitonic sort */
      #else
        printf("ERROR: You chose standard sorting on an iterative algorithm but int32_sort_interlaced is undefined. \n");
        exit(1);
      #endif
    #else
      printf("ERROR: You chose an iterative algorithm but no sorting type. \n");
      exit(1);
    #endif
  }

#elif defined(CHOOSE_CBRECURSION)
  static void int32_sort(int32_t *x, long long n) {
    #if defined(CHOOSE_DJBSORT2019)
      djbsort(x, n);
    #elif defined(CHOOSE_DJBSORT2026)
      djbsort(x, n);
    #elif defined(CHOOSE_BITONIC)
      bitonic(x, n);
    #elif defined(CHOOSE_PORTABLE4)
      portable4(x, n);
    #elif defined(CHOOSE_IBITONIC)
      char m = 0;
        while (n > 1) {
            n >>= 1; 
            m++;
        }
      ibitonic(x, m, 0);
    #else
      printf("ERROR: You chose standard sorting with cbrecursion but int32_sort is undefined. \n");
      exit(1);
    #endif
  }
#endif







