#include "director.h"
#include "../../src/utilities/utilities.h"
#include "../../src/utilities/portable_macros.h"


/* You need to include your path to valgrind's memcheck here. */
#include "/snap/valgrind/177/usr/include/valgrind/memcheck.h"

#if defined(CHOOSE_CBRECURSION)
  #include "../../src/controlbits/cbrecursion.h"
#elif defined(CHOOSE_CBITERATIVE)
  #include "../../src/controlbits/cbiterative.h"
#elif defined(CHOOSE_CBITERAVX)
  #include "../../src/controlbits/cbiteravx.h"
#elif defined(CHOOSE_CBITERATIVE_UNROLLED)
  #include "../../src/controlbits/cbiterative_unrolled.h"
#endif

void poison(void *x,unsigned long long xlen)
{
  (void) x;
  (void) xlen;
  VALGRIND_MAKE_MEM_UNDEFINED(x,xlen);
}

void unpoison(void *x,unsigned long long xlen)
{
  (void) x;
  (void) xlen;
  VALGRIND_MAKE_MEM_DEFINED(x,xlen);
}

void controlbits(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp){
  #if defined(CHOOSE_CBRECURSION)
    cbrecursion(out, 0, 1, pi, m, n, temp);
  #elif defined(CHOOSE_CBITERATIVE)
    cbiterative(out, pi, m, n, temp);
  #elif defined(CHOOSE_CBITERAVX)
    cbiteravx(out, pi, m, n, temp);
  #elif defined(CHOOSE_CBITERATIVE_UNROLLED)
    cbiterative_unrolled(out, pi, m, n, temp);
  #else
    printf("ERROR:\n Controlbits called without any choice defined. \n");
    exit(1);
  #endif
}

/* Valgrind *should* detect all errors in a single run. */
#define ROUNDS 1

int16_t perms[(ROUNDS+1) * N]                                   __attribute__((aligned(4096)));
int16_t correctness_check[(ROUNDS+1) * N]                       __attribute__((aligned(4096)));
unsigned char cb_outputs[(((2*M-1)*(N/2))+7)/8 * (ROUNDS+1)]    __attribute__((aligned(4096)));


int main() {
  int32_t i, round;
  /* Create ROUNDS+1 random permutations of size N. Store correct perms. */
  for (round=0; round < ROUNDS+1; round++) {
    for (i=0; i < N; i++) perms[N*round + i] = i;
    shuffle_int16(&perms[N*round], N, round);
    for (i=0; i < N; i++) correctness_check[N*round + i] = perms[N*round + i];
  }

  poison(&perms, (ROUNDS+1) * N);

  /* Initialise all cb_outputs to zero. */ 
  memset(cb_outputs, 0, (((2*M-1)*(N/2))+7)/8 * (ROUNDS+1));

  /* Initialise some temporary storage for the algorithms. */
  int32_t *temp = malloc(sizeof(int32_t) * 2 * N);

  /* Do rounds. */
  for (round=0; round < ROUNDS+1; round++) {
    controlbits( &cb_outputs[(((2*M-1)*(N/2))+7)/8 * round], &perms[N*round], M, N, temp);
  }

  
  unpoison(&perms, (ROUNDS+1) * N);
  unpoison(&correctness_check,(ROUNDS+1) * N);
  unpoison(&cb_outputs, (((2*M-1)*(N/2))+7)/8 * (ROUNDS+1));
    

  /* Check all control bits are correct. */
  for (round=0; round < ROUNDS+1; round++) {
    if ( check_cb_correctness(&correctness_check[N*round], N, &cb_outputs[(((2*M-1)*(N/2))+7)/8 * round]) == 0 ) 
    {
      printf("Control bit algorithm is incorrect. \n");
      exit(1);
    }
  }

  free(temp);
}