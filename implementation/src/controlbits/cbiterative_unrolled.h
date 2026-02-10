#ifndef ITERATIVE_SCALAR_UNROLLED
#define ITERATIVE_SCALAR_UNROLLED

#include "../utilities/portable_macros.h"
#include "../cryptoint/crypto_int32.h"
#define int32_min crypto_int32_min
#define int32_MINMAX(a,b) crypto_int32_minmax(&(a),&(b))
#include "../sorting/sort.h"

/* This inline function takes over the control bits logic temporarily provided that: 
  
      A = x0 = (id << 16) +  pibar_i
      B = anything

    then it will return:

      A = (id<< 16) + pi^-1 
      B = (0 << 16) + fj << w
    
  where fj is the jth control bit of f.
*/
static inline void cyclemin_calc(int16_t* pi, int32_t m, int32_t n, int32_t w, int32_t* temp) {
  #define A temp
  #define B (temp+n)
  int32_t x, j;
  /* Calculate c_1. */
  /* Set B = (pibar << 16) + c_1*/
  for (x = 0;x < n;++x) {
    int32_t Ax = A[x];      // Ax = (x << 16) + pibar(x) 
    int32_t px = Ax&0xffff; // px = pibar_i(x)
    int32_t cx = px;        // cx = pibar_i(x)
    if (x < cx) cx = x;     // cx = min(x, pibar(x)) 
    B[x] = (px<<16)|cx;     // B[x] = (pibar(x), c_1(x))
  }
  
  /* Set A = (id<<16) + pibar^-1 */
  for (x = 0;x < n;++x) A[x] = (A[x]<<16) | x; 
  int32_sort_interlaced(A,m, w); 

  /* Set A = (id<<16) + pibar^2 */
  for (x = 0;x < n;++x) A[x] = (A[x]<<16)+(B[x]>>16); /* A = (pibar^(-1)<<16)+pibar */
  int32_sort_interlaced(A,m, w);

  /* Set B = (pibar^2 << 16) + c */
  for (x = 0;x < n;++x) B[x] = (A[x]<<16)|(B[x]&0xffff);

  /* Calculate c_2, ..., c_{m-w-3}. */
  /* We always begin this loop with B = (p<<16) + c */
  for (j = 1; j < m-w-2 ;++j) {

    /* Set A = (id << 16) + p^-1 */
    for (x = 0;x < n;++x) A[x] = (B[x]&~0xffff)|x;
    int32_sort_interlaced(A,m,w); 
    
    /* Set A = (p^-1 << 16) + c */
    for (x = 0;x < n;++x) A[x] = (A[x]<<16)|(B[x]&0xffff);

    /* Set B = (id<<16)+p^2 */
    for (x = 0;x < n;++x) B[x] = (A[x]&~0xffff)|(B[x]>>16);  
    int32_sort_interlaced(B,m,w); 

    /* Set B = (p^2<<16)+c */
    for (x = 0;x < n;++x) B[x] = (B[x]<<16)|(A[x]&0xffff);
    
    /* Set A = (id<<16) + cp */
    int32_sort_interlaced(A,m,w);
    
    /* Set B = (p^2 << 16) + minimum of c and cp */
    for (x = 0;x < n;++x) {
      int32_t cpx = (B[x]&~0xffff) | (A[x]&0xffff);
      B[x] = int32_min(cpx, B[x]);
    }
  }

  /* Calculate c_{m-w-2}. Case seperated to remove calculation of p^2. */
  /* Set A = (id << 16) + p^-1 */
  for (x = 0;x < n;++x) A[x] = (B[x]&~0xffff)|x;
  int32_sort_interlaced(A,m,w); 
  
  /* Set A = (p^-1 << 16) + c */
  for (x = 0;x < n;++x) A[x] = (A[x]<<16)|(B[x]&0xffff);

  /* Set A = (id<<16) + cp */
  int32_sort_interlaced(A,m,w);
  
  /* Set B = (p^2 << 16) + cyclemin */
  /* Set A = (pi  << 16) + id */
  for (x = 0;x < n;++x) {
    int32_t cpx = (B[x]&~0xffff) | (A[x]&0xffff);
    B[x] = int32_min(cpx, B[x]);
    A[x] = (((int32_t)pi[x])<<16)+x;
  } 
}


/* This is an unrolled non-recursive implementation of the Nassimi-Sahni control bits algorithm.

Parameters:
  *out: 0-initialised space for (2m-1)n/2 control bits
  *pi: permutation of {0, 1, ..., n-1}
  *temp: must have space for int32_t[2n]
  4 <= m <= 14
  n = 2^m

Output:
  Control bits for the permutation pi.
  The nth control bit is defined to be 1&(out[n/8] >> (n&7))
*/
static void cbiterative_unrolled(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp) {
  int32_t w, j, x, k;

  /* Unroll w=0 case. */
  int32_t jump = 1;
  int32_t pos = 0;
  w = 0;
  /* Set A = (id << 16) + pibar_0 */ 
  for (x = 0; x < n; ++x) A[x] = ( (pi[x]^jump) << 16 ) | pi[x^jump];
  int32_sort_interlaced(A,m,w); 
  
  /* Set A = (id<< 16) + pi^-1 
         B = (0 << 16) + fj << w  */
  cyclemin_calc(pi, m, n, w, temp);

  /* Set A = (id<<16) + pi^-1 */
  int32_sort_interlaced(A,m,w); 

  /* Calculate and set f and F. */
  for (x = 0; x < n; x+=16) {

    /* The controlbit f_j is the LSB of cyclemin(2j), which is currently
       stored in B. */
    int32_t f0 = B[x   ] & 1;
    int32_t f1 = B[x+ 2] & 1;
    int32_t f2 = B[x+ 4] & 1;
    int32_t f3 = B[x+ 6] & 1;
    int32_t f4 = B[x+ 8] & 1;
    int32_t f5 = B[x+10] & 1;
    int32_t f6 = B[x+12] & 1;
    int32_t f7 = B[x+14] & 1;
    
    unsigned char controlbyte = 0;
    controlbyte ^= f0 << 0;
    controlbyte ^= f1 << 1;
    controlbyte ^= f2 << 2;
    controlbyte ^= f3 << 3;
    controlbyte ^= f4 << 4;
    controlbyte ^= f5 << 5;
    controlbyte ^= f6 << 6;
    controlbyte ^= f7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    /* Setup for calculating F(pi) in B*/
    /* Set B = (pi^-1 << 16) + F */
    int32_t Fx0   = x +  0 + f0;
    int32_t Fx2   = x +  2 + f1;
    int32_t Fx4   = x +  4 + f2;
    int32_t Fx6   = x +  6 + f3;
    int32_t Fx8   = x +  8 + f4;
    int32_t Fx10  = x + 10 + f5;
    int32_t Fx12  = x + 12 + f6;
    int32_t Fx14  = x + 14 + f7;

    int32_t Fx1   = Fx0  ^ 1;
    int32_t Fx3   = Fx2  ^ 1;
    int32_t Fx5   = Fx4  ^ 1;
    int32_t Fx7   = Fx6  ^ 1;
    int32_t Fx9   = Fx8  ^ 1;
    int32_t Fx11  = Fx10 ^ 1;
    int32_t Fx13  = Fx12 ^ 1;
    int32_t Fx15  = Fx14 ^ 1;

    B[x+0]      = (A[x+0]  << 16)    | Fx0;
    B[x+1]      = (A[x+1]  << 16)    | Fx1;
    B[x+2]      = (A[x+2]  << 16)    | Fx2;
    B[x+3]      = (A[x+3]  << 16)    | Fx3;
    B[x+4]      = (A[x+4]  << 16)    | Fx4;
    B[x+5]      = (A[x+5]  << 16)    | Fx5;
    B[x+6]      = (A[x+6]  << 16)    | Fx6;
    B[x+7]      = (A[x+7]  << 16)    | Fx7;
    B[x+8]      = (A[x+8]  << 16)    | Fx8;
    B[x+9]      = (A[x+9]  << 16)    | Fx9;
    B[x+10]     = (A[x+10] << 16)   | Fx10;
    B[x+11]     = (A[x+11] << 16)   | Fx11;
    B[x+12]     = (A[x+12] << 16)   | Fx12;
    B[x+13]     = (A[x+13] << 16)   | Fx13;
    B[x+14]     = (A[x+14] << 16)   | Fx14;
    B[x+15]     = (A[x+15] << 16)   | Fx15;
  }

  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(B,m,w);

  /* Set pi = Fpi*/
  for (x = 0;x < n; ++x) pi[x] = (int16_t) B[x];

  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-2-w) * (n >> 4);
  for (x = 0; x < n; x+=16) {

    /* The controlbit f_j is the LSB of Fpi(2j), which is currently
       stored in B. */
    int32_t l0 = pi[x   ] & 1;
    int32_t l1 = pi[x+ 2] & 1;
    int32_t l2 = pi[x+ 4] & 1;
    int32_t l3 = pi[x+ 6] & 1;
    int32_t l4 = pi[x+ 8] & 1;
    int32_t l5 = pi[x+10] & 1;
    int32_t l6 = pi[x+12] & 1;
    int32_t l7 = pi[x+14] & 1;

    unsigned char controlbyte = 0;
    controlbyte ^= l0 << 0;
    controlbyte ^= l1 << 1;
    controlbyte ^= l2 << 2;
    controlbyte ^= l3 << 3;
    controlbyte ^= l4 << 4;
    controlbyte ^= l5 << 5;
    controlbyte ^= l6 << 6;
    controlbyte ^= l7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    APPLY_CONTROL_BIT(pi[x+0],pi[x+1], l0);
    APPLY_CONTROL_BIT(pi[x+2],pi[x+3], l1);
    APPLY_CONTROL_BIT(pi[x+4],pi[x+5], l2);
    APPLY_CONTROL_BIT(pi[x+6],pi[x+7], l3);
    APPLY_CONTROL_BIT(pi[x+8],pi[x+9], l4);
    APPLY_CONTROL_BIT(pi[x+10],pi[x+11], l5);
    APPLY_CONTROL_BIT(pi[x+12],pi[x+13], l6);
    APPLY_CONTROL_BIT(pi[x+14],pi[x+15], l7);

  }
  
  /* Unroll w=1 case. */
  jump <<= 1;
  w++;

  /* Set A = (id << 16) + pibar_i */ 
  for (x = 0; x < n; ++x) A[x] = ( (pi[x]^jump) << 16 ) | pi[x^jump];
  int32_sort_interlaced(A,m,w); 
  
  cyclemin_calc(pi, m, n, w, temp);

  /* Set A = (id<<16) + pi^-1 */
  int32_sort_interlaced(A,m,w); 

  /* Calculate and set f and F. */
  pos = w * (n >> 4);
  for (x = 0; x < n; x+=16) {

    /* The controlbit f_j is the 2nd LSB of cyclemin(4j) and cyclmin(4j+1). */
    int32_t f0 = (B[x   ] >> w) & 1;
    int32_t f1 = (B[x+ 1] >> w) & 1;
    int32_t f2 = (B[x+ 4] >> w) & 1;
    int32_t f3 = (B[x+ 5] >> w) & 1;
    int32_t f4 = (B[x+ 8] >> w) & 1;
    int32_t f5 = (B[x+ 9] >> w) & 1;
    int32_t f6 = (B[x+12] >> w) & 1;
    int32_t f7 = (B[x+13] >> w) & 1;
    
    unsigned char controlbyte = 0;
    controlbyte ^= f0 << 0;
    controlbyte ^= f1 << 1;
    controlbyte ^= f2 << 2;
    controlbyte ^= f3 << 3;
    controlbyte ^= f4 << 4;
    controlbyte ^= f5 << 5;
    controlbyte ^= f6 << 6;
    controlbyte ^= f7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    /* Setup for calculating F(pi) in B*/
    /* Set B = (pi^-1 << 16) + F */
    int32_t Fx0   = x +  0 + (f0 << w);
    int32_t Fx1   = x +  1 + (f1 << w);
    int32_t Fx4   = x +  4 + (f2 << w);
    int32_t Fx5   = x +  5 + (f3 << w);
    int32_t Fx8   = x +  8 + (f4 << w);
    int32_t Fx9   = x +  9 + (f5 << w);
    int32_t Fx12  = x + 12 + (f6 << w);
    int32_t Fx13  = x + 13 + (f7 << w);

    int32_t Fx2   = Fx0  ^ jump;
    int32_t Fx3   = Fx1  ^ jump;
    int32_t Fx6   = Fx4  ^ jump;
    int32_t Fx7   = Fx5  ^ jump;
    int32_t Fx10  = Fx8  ^ jump;
    int32_t Fx11  = Fx9  ^ jump;
    int32_t Fx14  = Fx12 ^ jump;
    int32_t Fx15  = Fx13 ^ jump;

    B[x+0]      = (A[x+0]  << 16)    | Fx0;
    B[x+1]      = (A[x+1]  << 16)    | Fx1;
    B[x+2]      = (A[x+2]  << 16)    | Fx2;
    B[x+3]      = (A[x+3]  << 16)    | Fx3;
    B[x+4]      = (A[x+4]  << 16)    | Fx4;
    B[x+5]      = (A[x+5]  << 16)    | Fx5;
    B[x+6]      = (A[x+6]  << 16)    | Fx6;
    B[x+7]      = (A[x+7]  << 16)    | Fx7;
    B[x+8]      = (A[x+8]  << 16)    | Fx8;
    B[x+9]      = (A[x+9]  << 16)    | Fx9;
    B[x+10]     = (A[x+10] << 16)    | Fx10;
    B[x+11]     = (A[x+11] << 16)    | Fx11;
    B[x+12]     = (A[x+12] << 16)    | Fx12;
    B[x+13]     = (A[x+13] << 16)    | Fx13;
    B[x+14]     = (A[x+14] << 16)    | Fx14;
    B[x+15]     = (A[x+15] << 16)    | Fx15;
  }

  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(B,m,w);

  /* Set pi = Fpi*/
  for (x = 0;x < n; ++x) pi[x] = (int16_t) B[x];

  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-2-w) * (n >> 4);
  for (x = 0; x < n; x+=16) {
    
    /* The controlbit l_j is the 2nd LSB of Fpi(4j) and Fpi(4j+1). */
    int32_t l0 = (pi[x   ] >> w) & 1;
    int32_t l1 = (pi[x+ 1] >> w) & 1;
    int32_t l2 = (pi[x+ 4] >> w) & 1;
    int32_t l3 = (pi[x+ 5] >> w) & 1;
    int32_t l4 = (pi[x+ 8] >> w) & 1;
    int32_t l5 = (pi[x+ 9] >> w) & 1;
    int32_t l6 = (pi[x+12] >> w) & 1;
    int32_t l7 = (pi[x+13] >> w) & 1;
    
    unsigned char controlbyte = 0;
    controlbyte ^= l0 << 0;
    controlbyte ^= l1 << 1;
    controlbyte ^= l2 << 2;
    controlbyte ^= l3 << 3;
    controlbyte ^= l4 << 4;
    controlbyte ^= l5 << 5;
    controlbyte ^= l6 << 6;
    controlbyte ^= l7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    APPLY_CONTROL_BIT(pi[x+0], pi[x+2],  l0);
    APPLY_CONTROL_BIT(pi[x+1], pi[x+3],  l1);
    APPLY_CONTROL_BIT(pi[x+4], pi[x+6],  l2);
    APPLY_CONTROL_BIT(pi[x+5], pi[x+7],  l3);
    APPLY_CONTROL_BIT(pi[x+8], pi[x+10], l4);
    APPLY_CONTROL_BIT(pi[x+9], pi[x+11], l5);
    APPLY_CONTROL_BIT(pi[x+12],pi[x+14], l6);
    APPLY_CONTROL_BIT(pi[x+13],pi[x+15], l7);
  }

  /* Unroll w=2 case. */
  jump <<= 1;
  w++;

  /* Set A = (id << 16) + pibar_i */ 
  for (x = 0; x < n; ++x) A[x] = ( (pi[x]^jump) << 16 ) | pi[x^jump];
  int32_sort_interlaced(A,m,w); 
  
  cyclemin_calc(pi, m, n, w, temp);

  /* Set A = (id<<16) + pi^-1 */
  int32_sort_interlaced(A,m,w); 

  /* Calculate and set f and F. */

  /* Calculate and set f and F. */
  pos = w * (n >> 4);
  for (x = 0; x < n; x+=16) {

    /* The controlbit f_j is the 3rd LSB of cyclemin(4j+i) where i=0,1,2,3. */
    int32_t f0 = (B[x   ] >> w) & 1;
    int32_t f1 = (B[x+ 1] >> w) & 1;
    int32_t f2 = (B[x+ 2] >> w) & 1;
    int32_t f3 = (B[x+ 3] >> w) & 1;
    int32_t f4 = (B[x+ 8] >> w) & 1;
    int32_t f5 = (B[x+ 9] >> w) & 1;
    int32_t f6 = (B[x+10] >> w) & 1;
    int32_t f7 = (B[x+11] >> w) & 1;
    
    unsigned char controlbyte = 0;
    controlbyte ^= f0 << 0;
    controlbyte ^= f1 << 1;
    controlbyte ^= f2 << 2;
    controlbyte ^= f3 << 3;
    controlbyte ^= f4 << 4;
    controlbyte ^= f5 << 5;
    controlbyte ^= f6 << 6;
    controlbyte ^= f7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    /* Setup for calculating F(pi) in B*/
    /* Set B = (pi^-1 << 16) + F */
    int32_t Fx0   = x +  0 + (f0 << w);
    int32_t Fx1   = x +  1 + (f1 << w);
    int32_t Fx2   = x +  2 + (f2 << w);
    int32_t Fx3   = x +  3 + (f3 << w);
    int32_t Fx8   = x +  8 + (f4 << w);
    int32_t Fx9   = x +  9 + (f5 << w);
    int32_t Fx10  = x + 10 + (f6 << w);
    int32_t Fx11  = x + 11 + (f7 << w);

    int32_t Fx4   = Fx0  ^ jump;
    int32_t Fx5   = Fx1  ^ jump;
    int32_t Fx6   = Fx2  ^ jump;
    int32_t Fx7   = Fx3  ^ jump;
    int32_t Fx12  = Fx8  ^ jump;
    int32_t Fx13  = Fx9  ^ jump;
    int32_t Fx14  = Fx10 ^ jump;
    int32_t Fx15  = Fx11 ^ jump;

    B[x+0]      = (A[x+0]  << 16)    | Fx0;
    B[x+1]      = (A[x+1]  << 16)    | Fx1;
    B[x+2]      = (A[x+2]  << 16)    | Fx2;
    B[x+3]      = (A[x+3]  << 16)    | Fx3;
    B[x+4]      = (A[x+4]  << 16)    | Fx4;
    B[x+5]      = (A[x+5]  << 16)    | Fx5;
    B[x+6]      = (A[x+6]  << 16)    | Fx6;
    B[x+7]      = (A[x+7]  << 16)    | Fx7;
    B[x+8]      = (A[x+8]  << 16)    | Fx8;
    B[x+9]      = (A[x+9]  << 16)    | Fx9;
    B[x+10]     = (A[x+10] << 16)    | Fx10;
    B[x+11]     = (A[x+11] << 16)    | Fx11;
    B[x+12]     = (A[x+12] << 16)    | Fx12;
    B[x+13]     = (A[x+13] << 16)    | Fx13;
    B[x+14]     = (A[x+14] << 16)    | Fx14;
    B[x+15]     = (A[x+15] << 16)    | Fx15;
  }


  /* Set B = (id<<16) + F(pi) */
  int32_sort_interlaced(B,m,w);

  /* Set pi = Fpi*/
  for (x = 0;x < n; ++x) pi[x] = (int16_t) B[x];

  /* Calculate l and set pi = M = FpiL. */
  pos = (2*m-2-w) * (n >> 4);
  for (x = 0; x < n; x+=16) {

    /* The controlbit l_j is the 3rd LSB of Fpi(4j+i) where i=0,1,2,3. */
    int32_t l0 = (pi[x   ] >> w) & 1;
    int32_t l1 = (pi[x+ 1] >> w) & 1;
    int32_t l2 = (pi[x+ 2] >> w) & 1;
    int32_t l3 = (pi[x+ 3] >> w) & 1;
    int32_t l4 = (pi[x+ 8] >> w) & 1;
    int32_t l5 = (pi[x+ 9] >> w) & 1;
    int32_t l6 = (pi[x+10] >> w) & 1;
    int32_t l7 = (pi[x+11] >> w) & 1;
    
    unsigned char controlbyte = 0;
    controlbyte ^= l0 << 0;
    controlbyte ^= l1 << 1;
    controlbyte ^= l2 << 2;
    controlbyte ^= l3 << 3;
    controlbyte ^= l4 << 4;
    controlbyte ^= l5 << 5;
    controlbyte ^= l6 << 6;
    controlbyte ^= l7 << 7;
    
    out[pos] = controlbyte;
    pos++;

    APPLY_CONTROL_BIT(pi[x+0], pi[x+4],  l0);
    APPLY_CONTROL_BIT(pi[x+1], pi[x+5],  l1);
    APPLY_CONTROL_BIT(pi[x+2], pi[x+6],  l2);
    APPLY_CONTROL_BIT(pi[x+3], pi[x+7],  l3);
    APPLY_CONTROL_BIT(pi[x+8], pi[x+12], l4);
    APPLY_CONTROL_BIT(pi[x+9], pi[x+13], l5);
    APPLY_CONTROL_BIT(pi[x+10],pi[x+14], l6);
    APPLY_CONTROL_BIT(pi[x+11],pi[x+15], l7);
  }

  
  /* Now begin general case. */
  for (w = 3; w < m-1; w++) {
    jump = (1 << w);

    /* Set A = (id << 16) + pibar_i */ 
    for (x = 0; x < n; ++x) A[x] = ( (pi[x]^jump) << 16 ) | pi[x^jump];
    int32_sort_interlaced(A,m,w); 
    
    cyclemin_calc(pi, m, n, w, temp);

    /* Set A = (id<<16) + pi^-1 */
    int32_sort_interlaced(A,m,w); 

    /* Calculate and set f and F. */
    pos = w * (n >> 4);
    for (j=0; j < n; j += 2*jump) {
      for (k=0; k < jump; k+=8) {
        /* The controlbit f_i is the wth LSB of cyclemin(j+k+i) where i=0,1,2,... */
        int32_t f0 = (B[j+k  ] >> w) & 1;
        int32_t f1 = (B[j+k+1] >> w) & 1;
        int32_t f2 = (B[j+k+2] >> w) & 1;
        int32_t f3 = (B[j+k+3] >> w) & 1;
        int32_t f4 = (B[j+k+4] >> w) & 1;
        int32_t f5 = (B[j+k+5] >> w) & 1;
        int32_t f6 = (B[j+k+6] >> w) & 1;
        int32_t f7 = (B[j+k+7] >> w) & 1;

        unsigned char controlbyte = 0;
        controlbyte ^= f0 << 0;
        controlbyte ^= f1 << 1;
        controlbyte ^= f2 << 2;
        controlbyte ^= f3 << 3;
        controlbyte ^= f4 << 4;
        controlbyte ^= f5 << 5;
        controlbyte ^= f6 << 6;
        controlbyte ^= f7 << 7;
        
        out[pos] = controlbyte;
        pos++;

        int32_t Fx0 = (j+k+ 0) ^ (f0 << w);
        int32_t Fx1 = (j+k+ 1) ^ (f1 << w);
        int32_t Fx2 = (j+k+ 2) ^ (f2 << w);
        int32_t Fx3 = (j+k+ 3) ^ (f3 << w);
        int32_t Fx4 = (j+k+ 4) ^ (f4 << w);
        int32_t Fx5 = (j+k+ 5) ^ (f5 << w);
        int32_t Fx6 = (j+k+ 6) ^ (f6 << w);
        int32_t Fx7 = (j+k+ 7) ^ (f7 << w);

        int32_t Fx0plus_jump  = Fx0 ^ jump;
        int32_t Fx1plus_jump  = Fx1 ^ jump;
        int32_t Fx2plus_jump  = Fx2 ^ jump;
        int32_t Fx3plus_jump  = Fx3 ^ jump;
        int32_t Fx4plus_jump  = Fx4 ^ jump;
        int32_t Fx5plus_jump  = Fx5 ^ jump;
        int32_t Fx6plus_jump  = Fx6 ^ jump;
        int32_t Fx7plus_jump  = Fx7 ^ jump;

        B[j+k+0] = (A[j+k+0] << 16 ) | Fx0;
        B[j+k+1] = (A[j+k+1] << 16 ) | Fx1;
        B[j+k+2] = (A[j+k+2] << 16 ) | Fx2;
        B[j+k+3] = (A[j+k+3] << 16 ) | Fx3;
        B[j+k+4] = (A[j+k+4] << 16 ) | Fx4;
        B[j+k+5] = (A[j+k+5] << 16 ) | Fx5;
        B[j+k+6] = (A[j+k+6] << 16 ) | Fx6;
        B[j+k+7] = (A[j+k+7] << 16 ) | Fx7;

        B[j+k+jump+0] = (A[j+k+jump+0] << 16 ) | Fx0plus_jump;
        B[j+k+jump+1] = (A[j+k+jump+1] << 16 ) | Fx1plus_jump;
        B[j+k+jump+2] = (A[j+k+jump+2] << 16 ) | Fx2plus_jump;
        B[j+k+jump+3] = (A[j+k+jump+3] << 16 ) | Fx3plus_jump;
        B[j+k+jump+4] = (A[j+k+jump+4] << 16 ) | Fx4plus_jump;
        B[j+k+jump+5] = (A[j+k+jump+5] << 16 ) | Fx5plus_jump;
        B[j+k+jump+6] = (A[j+k+jump+6] << 16 ) | Fx6plus_jump;
        B[j+k+jump+7] = (A[j+k+jump+7] << 16 ) | Fx7plus_jump;
      }
    }

    /* Set B = (id<<16) + F(pi) */
    int32_sort_interlaced(B,m,w);

    /* Set pi = Fpi*/
    for (x = 0;x < n; ++x) pi[x] = (int16_t) B[x];

    /* Calculate l and set pi = M = FpiL. */
    pos = (2*m-2-w) * (n >> 4);
    for (j=0; j < n; j += 2*jump) {
      for (k=0; k < jump; k+=8) {
        /* The controlbit f_i is the wth LSB of Fpi(j+k+i) where i=0,1,2,... */
        int32_t l0 = (pi[j+k  ] >> w) & 1;
        int32_t l1 = (pi[j+k+1] >> w) & 1;
        int32_t l2 = (pi[j+k+2] >> w) & 1;
        int32_t l3 = (pi[j+k+3] >> w) & 1;
        int32_t l4 = (pi[j+k+4] >> w) & 1;
        int32_t l5 = (pi[j+k+5] >> w) & 1;
        int32_t l6 = (pi[j+k+6] >> w) & 1;
        int32_t l7 = (pi[j+k+7] >> w) & 1;

        unsigned char controlbyte = 0;
        controlbyte ^= l0 << 0;
        controlbyte ^= l1 << 1;
        controlbyte ^= l2 << 2;
        controlbyte ^= l3 << 3;
        controlbyte ^= l4 << 4;
        controlbyte ^= l5 << 5;
        controlbyte ^= l6 << 6;
        controlbyte ^= l7 << 7;
        
        out[pos] = controlbyte;
        pos++;

        APPLY_CONTROL_BIT(pi[j+k+0], pi[j+k+jump+0],  l0);
        APPLY_CONTROL_BIT(pi[j+k+1], pi[j+k+jump+1],  l1);
        APPLY_CONTROL_BIT(pi[j+k+2], pi[j+k+jump+2],  l2);
        APPLY_CONTROL_BIT(pi[j+k+3], pi[j+k+jump+3],  l3);
        APPLY_CONTROL_BIT(pi[j+k+4], pi[j+k+jump+4],  l4);
        APPLY_CONTROL_BIT(pi[j+k+5], pi[j+k+jump+5],  l5);
        APPLY_CONTROL_BIT(pi[j+k+6], pi[j+k+jump+6],  l6);
        APPLY_CONTROL_BIT(pi[j+k+7], pi[j+k+jump+7],  l7);

      }
    }
  }

  /* Calculate the middle-most control bits. */
  pos = (m-1) * (n >> 4);
  jump = (1 << (m-1));
  for (j =0; j < n/2; j += 8) {
    unsigned char controlbyte = 0;

    /* The controlbit m_i is the MSB of FpiL(x) where 0<=i<=n/2. */
    int32_t m0 = pi[j  ] >> (m-1);
    int32_t m1 = pi[j+1] >> (m-1);
    int32_t m2 = pi[j+2] >> (m-1);
    int32_t m3 = pi[j+3] >> (m-1);
    int32_t m4 = pi[j+4] >> (m-1);
    int32_t m5 = pi[j+5] >> (m-1);
    int32_t m6 = pi[j+6] >> (m-1);
    int32_t m7 = pi[j+7] >> (m-1);

    /* Set the control bit in the output.*/
    controlbyte ^= m0 << 0;
    controlbyte ^= m1 << 1;
    controlbyte ^= m2 << 2;
    controlbyte ^= m3 << 3;
    controlbyte ^= m4 << 4;
    controlbyte ^= m5 << 5;
    controlbyte ^= m6 << 6;
    controlbyte ^= m7 << 7;
    
    out[pos] = controlbyte;
    pos++;
  }
}
#endif