#ifndef ITERATIVE_SCALAR
#define ITERATIVE_SCALAR

#include "../utilities/portable_macros.h"
#include "../cryptoint/crypto_int32.h"
#include "../sorting/sort.h"
#define int32_min crypto_int32_min

/* This is a non-recursive implementation of the Nassimi-Sahni algorithm for calculating 
   the control bits for a Bene\v{s} network.

Parameters:
  *out: 0-initialised space for (2m-1)n/2 control bits
  *pi: permutation of {0, 1, ..., n-1}
  *temp: must have space for int32_t[2n]
  1 <= m <= 14
  n = 2^m

Output:
  Control bits for the permutation pi.
  The nth control bit is defined to be 1&(out[n/8] >> (n&7))
*/
static void cbiterative(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp) {
  #define A temp
  #define B (temp+n)
  int32_t w, j, x, k;

  for (w = 0; w < m-1; w++) {
    int32_t jump = (1 << w);

    /* Set A = (id << 16) + pibar_i */ 
    for (x = 0; x < n; ++x) A[x] = ( (pi[x]^jump) << 16 ) | pi[x^jump];
    int32_sort_interlaced(A,m,w); 

    /* Calculate c_1. */
    /* Set B = (pibar << 16) + c_1*/
    for (x = 0;x < n;++x) {
      int32_t Ax = A[x];      // Ax = (x << 16) + pibar(x) 
      int32_t px = Ax&0xffff; // px = pibar_i(x)
      int32_t cx = int32_min(px,x);
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

    /* Set A = (id<<16) + pi^-1 */
    int32_sort_interlaced(A,m,w); 

    /* Calculate and set f and F. */
    int32_t pos = w * (n >> 1);
    for (j = 0; j < n; j+= 2* jump) {
      for (k=0; k < jump; k++) {

        /* Set f_j = ith bit of cyclemin(j+k) */ 
        int32_t fj = crypto_int32_bottombit_01(B[j+k] >> w);  

        /* Set the control bit in the output.*/
        out[pos>>3] ^= fj<<(pos&7);
        pos += 1;

        /* Set F[j+k], F[j+k+jump] */
        int32_t Fx = (j+k) ^ (fj << w); 
        int32_t Fxxor2i = Fx ^ jump; 
        
        /* Setup for calculating F(pi) in B*/
        /* Set B = (pi^-1 << 16) + F */
        B[j+k]      = (A[j+k]<<16)       | Fx;
        B[j+k+jump] = (A[j+k+jump]<<16)  | Fxxor2i; 
      }
    }

    /* Set B = (id<<16) + F(pi) */
    int32_sort_interlaced(B,m,w);

    /* Set pi = Fpi*/
    for (x = 0;x < n; ++x) pi[x] = (int16_t) B[x];

    /* Calculate l and set pi = M = FpiL. */
    pos = (2*m-2-w) * (n >> 1);
    for (j = 0; j < n; j+= 2* jump) {
      for (k=0; k < jump; k++) {

        /* Set l_j = ith bit of F(pi((j+k)) */ 
        int32_t lj = crypto_int32_bottombit_01(pi[j+k] >> w);  

        /* Set the control bit in the output.*/
        out[pos>>3] ^= lj<<(pos&7);
        pos += 1;

        /* Set pi[x] = M(x) = FpiL(x)*/
        APPLY_CONTROL_BIT(pi[j+k],pi[j+k+jump], lj);
      }
    }
  }

  /* Calculate the middle-most control bits. */
  int32_t pos = (m-1) * (n >> 1);
  int32_t jump = (1 << (m-1));

  for (j =0; j < n/2; ++j) {

    int32_t fj = ( pi[j] & jump ) >> (m-1);

    /* Set the control bit in the output.*/
    out[pos>>3] ^= fj<<(pos&7);
    pos += 1;
  }
}
#endif