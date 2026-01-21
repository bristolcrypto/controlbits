
#include "director.h"
#include "../src/utilities/utilities.h"
#include <stdlib.h>

#if defined(CHOOSE_CBRECURSION)
  #include "../src/controlbits/cbrecursion.h"
#elif defined(CHOOSE_CBITERATIVE)
  #include "../src/controlbits/cbiterative.h"
#elif defined(CHOOSE_CBITERAVX)
  #include "../src/controlbits/cbiteravx.h"
#elif defined(CHOOSE_CBITERATIVE_UNROLLED)
  #include "../src/controlbits/cbiterative_unrolled.h"
#endif

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
    printf("ERROR: Controlbits called without any choice defined. \n");
    exit(1);
  #endif
}


/* Reads the x86 Time Stamp Counter. */
long long ticks(void)
{
  unsigned long long result;
  asm volatile(".byte 15; .byte 49; shlq $32, %%rdx;orq %%rdx,%%rax"
                : "=a"(result) :: "%rdx");
  return result;
}

int16_t perms[(ROUNDS+1) * N]                                   __attribute__((aligned(4096)));
int16_t correctness_check[(ROUNDS+1) * N]                       __attribute__((aligned(4096)));
unsigned char cb_outputs[(((2*M-1)*(N/2))+7)/8 * (ROUNDS+1)]    __attribute__((aligned(4096)));
long long recorded_times[ROUNDS+1]                              __attribute__((aligned(4096)));


int main() {
  int32_t i, round;
  /* Create ROUNDS+1 random permutations of size N. Store correct perms. */
  for (round=0; round < ROUNDS+1; round++) {
    for (i=0; i < N; i++) perms[N*round + i] = i;
    shuffle_int16(&perms[N*round], N, round);
    for (i=0; i < N; i++) correctness_check[N*round + i] = perms[N*round + i];
  }

  /* Initialise all cb_outputs to zero. */ 
  memset(cb_outputs, 0, (((2*M-1)*(N/2))+7)/8 * (ROUNDS+1));

  /* Initialise some temporary storage for the algorithms. */
  int32_t *temp = malloc(sizeof(int32_t) * 2 * N);

  /* Bernstein does this, I'm not sure why. */
  for (i=0; i < ROUNDS+1; i++)
    recorded_times[i] = ticks();

  /* Take measurements. */
  for (round=0; round < ROUNDS+1; round++) {
    recorded_times[round] = ticks();
    controlbits( &cb_outputs[(((2*M-1)*(N/2))+7)/8 * round], &perms[N*round], M, N, temp);
  }

  /* Check all control bits are correct. */
  for (round=0; round < ROUNDS+1; round++) {
    if ( check_cb_correctness(&correctness_check[N*round], N, &cb_outputs[(((2*M-1)*(N/2))+7)/8 * round]) == 0 ) 
    {
      printf("Control bit algorithm is incorrect. \n");
      exit(1);
    }
  }


  /* Convert time stamps to time differences. */
  for (i=0; i < ROUNDS; i++)
    recorded_times[i] = recorded_times[i+1]-recorded_times[i];

  /* Sort execution times. */
  for (long long i = 0;i < ROUNDS;++i)
    for (long long j = 0;j < i;++j)
      if (recorded_times[i] < recorded_times[j]) {
        long long z = recorded_times[i];
        recorded_times[i] = recorded_times[j];
        recorded_times[j] = z;
      }
  
  /* Print sorted execution times to stdout. */
  for (i=0; i < ROUNDS; i++)
    printf("%lld ", recorded_times[i]);
}