#ifndef UTILITIES
#define UTILITIES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "portable_macros.h"
#include "../cryptoint/crypto_int16.h"

void apply_controlbits(int16_t *p, int32_t n, unsigned char *cbs);
void shuffle_int16(int16_t *array, size_t n, int32_t seed);
void shuffle_int32(int32_t *array, size_t n, int32_t seed);
void print_permutation_int16(int16_t *perm, int32_t n);
void print_permutation_int32(int32_t *perm, int32_t n);
int32_t* create_input_to_stage(int32_t m, int32_t w);
int check_cb_correctness(int16_t *pi, int32_t n, unsigned char *cbs);
int check_perm_correctness(int32_t *result_of_sort, int32_t n);
void print_control_bits(unsigned char *cbs, int32_t m);
void print_every_kth(int32_t * x, int32_t n, int32_t k);


/* 
This function applies a permutation given by a set of control bits.

Inputs:
  p - the array of values to be permuted
  n - the size of the array
  cbs = the control bits to sort p with

*/
void apply_controlbits(int16_t *p, int32_t n, unsigned char *cbs) {
    
  int32_t i;
  int32_t pos = 0;
  int16_t jump, cb;
  
  int32_t m = 1;
  while ((n >> m) > 1) m++;

  /* Iterate over the layers */
  for (i = 0; i < 2*m - 1; ++i) {
      jump = 1 << crypto_int16_min(i, 2*m - 2 - i);

    /* Apply the control bits in the layer */
    for (int j = 0; j < n; j += jump*2) {
      for (int k=0; k< jump; k++) {
        cb = 1&(cbs[pos>>3]>>(pos&7));
        APPLY_CONTROL_BIT(p[j+k],p[j+k+jump], cb);
        pos++;
      }
    }
  }
}

/* 
This function applies a permutation given by a set of control bits and skips the
skips the control bits that are guaranteed to be zero

Inputs:
  p - the array of values to be permuted
  n - the size of the array
  cbs = the control bits to sort p with

*/
void apply_controlbits_space_saving(int16_t *p, int32_t n, unsigned char *cbs) {
    
  int32_t i;
  int32_t pos = 0;
  int16_t jump, cb;
  
  int32_t m = 1;
  while ((n >> m) > 1) m++;

  /* Iterate over the F layers */
  for (i = 0; i < m-1; ++i) {
    jump = 1 << i;
    /* Apply the control bits in the layer */
    pos += jump;
    for (int j = jump*2; j < n; j += jump*2) {
      for (int k=0; k< jump; k++) {
        cb = 1&(cbs[pos>>3]>>(pos&7));
        APPLY_CONTROL_BIT(p[j+k],p[j+k+jump], cb);
        pos++;
      }
    }
  }

  /* Iterate over the L layers */
  for (i = m-1; i < 2*m - 1; ++i) {
    jump = 1 << (2*m - 2 - i);
    /* Apply the control bits in the layer */
    for (int j = 0; j < n; j += jump*2) {
      for (int k=0; k< jump; k++) {
        cb = 1&(cbs[pos>>3]>>(pos&7));
        APPLY_CONTROL_BIT(p[j+k],p[j+k+jump], cb);
        pos++;
      }
    }
  }
}

void shuffle_int16(int16_t *array, size_t n, int32_t seed)
{
    srand(seed+time(NULL));
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void shuffle_int32(int32_t *array, size_t n, int32_t seed)
{
    srand(seed+time(NULL));
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void print_permutation_int16(int16_t *perm, int32_t n) {
    printf("["); 
    for (int x = 0; x < n-1; x++) printf("%d, ", perm[x]);
    printf("%d] \n", perm[n-1]);
}
void print_permutation_int32(int32_t *perm, int32_t n) {
    printf("["); 
    for (int x = 0; x < n-1; x++) printf("%d, ", perm[x]);
    printf("%d] \n", perm[n-1]);
}
void print_control_bits(unsigned char *cbs, int32_t m) {
    int32_t n = 1 << m;

    printf("[");
    for (int pos = 0; pos < n*(2*m - 1)/2 - 1; pos++) printf("%x, ", 1&(cbs[pos>>3]>>(pos&7)));
    printf("%d] \n", 1&(cbs[( n*(2*m - 1)/2 - 1 )>>3]>>(( n*(2*m - 1)/2 - 1 ) & 7)));
}

void print_every_kth(int32_t * x, int32_t n, int32_t k) {
  
  /* Set m = log_2(n), l = log_2(k) */
  int32_t m = 0;
  while ((n >> m) > 1) m++;

  int32_t l = 0;
  while ((k >> l) > 1) l++;

  int32_t temp[(1 << (m-l))];
  
  for ( int i = 0; i < k; i++ ) {
    int idx = 0;
    for (int j = i; j < n; j+=k) {
      temp[idx] = x[j];
      idx++;
    }
    print_permutation_int32(temp, (1 << (m-l)));
  }
}

void print_blocks_of_k(int32_t * x, int32_t n, int32_t k) {
  int32_t temp[k];
  for ( int i =0; i < n; i+=k) {
    for (int j = 0; j < k; j++ ) {
      temp[j] = x[i+j];
    }
    print_permutation_int32(temp, k);
  }
}

int32_t* create_input_to_stage(int32_t m, int32_t w) {
    /* Stage (m, w) has n = 2^m total elements. This is comprised of 
       2^w interlaced random permutations of size 2^(m-w).        */

    int32_t n = 1 << m;
    int32_t num_perms_to_interlace = 1 << w;
    int32_t size_of_each_perm = 1 << (m-w);

    int32_t *pi = (int32_t *)  malloc(sizeof(uint32_t) * n);
    int32_t temp[size_of_each_perm];

    for ( int32_t channel = 0; channel < num_perms_to_interlace; channel++) {
        
        /* Reset temp. */
        for (int x = 0; x < size_of_each_perm; x++) temp[x] = x*num_perms_to_interlace+channel;

        /* Shuffle temp */
        shuffle_int32(temp, size_of_each_perm, channel);
        
        /* Place temp in pi */
        for (int x = 0; x < size_of_each_perm; x++) pi[x*num_perms_to_interlace + channel] = temp[x];
    }

    return pi;
}


/* Given a permutation pi and control bits cbs, this function 
   returns 1 when applying the control bits gives the 
   permutation and 0 otherwise. */
int check_cb_correctness(int16_t *pi, int32_t n, unsigned char *cbs) {
    
    /* Initialise the identity permutation to apply cbs to. */
    int16_t pi_compare[n];
    for (int16_t x = 0; x < n; x++) pi_compare[x] = x;
    
    apply_controlbits_space_saving(pi_compare, n, cbs);

    for (int x = 0; x < n; x++) {
        if (pi_compare[x] != pi[x]) return 0;
    }

    return 1;
}

int check_perm_correctness(int32_t *result_of_sort, int32_t n) {
    for (int x = 0; x < n; x++) {
        if (x != result_of_sort[x]) return 0;
    }
    return 1;
}
#endif