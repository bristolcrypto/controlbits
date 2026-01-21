#ifndef IPERMSORT
#define IPERMSORT
#include "../../utilities/avx_macros.h"
#include "permsorts/djbsort.h"
#include "permsorts/iperm_w1.h"
#include "permsorts/iperm_w2.h"
#include "ibitonic.h"


static void ipermsort(int32_t* x, int32_t m, int32_t w){
  if (m < 6) {
    printf("Error, ipermsort called with m < 6.\n");
    exit(1);
  }

  if ( w <= 0 ) {
    djbsort(x, 1 << m);
  } else if ( w <= 1 ) {
    iperm_w1(x, m);
  } else if ( w <= 2 ) {
    iperm_w2(x, m);
  } else {
    ibitonic_w_geq_3(x, m, w);
  }
}
#endif
