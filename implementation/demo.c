#include "src/utilities/utilities.h"


#define CHOOSE_CBITERATIVE
#define CHOOSE_PORTABLE4
#define CHOOSE_INTERLACED_SORTING



#if defined(CHOOSE_CBITERAVX)
  #include "src/controlbits/cbiteravx.h"
#elif defined(CHOOSE_CBITERATIVE)
  #include "src/controlbits/cbiterative.h"
#elif defined(CHOOSE_CBRECURSION)
  #include "src/controlbits/cbrecursion.h"
#elif defined(CHOOSE_CBITERATIVE_UNROLLED)
  #include "src/controlbits/cbiterative_unrolled.h"
#endif

void controlbits_alg(unsigned char *out, int16_t *pi, int32_t m, int32_t n, int32_t *temp) {
  #ifdef CHOOSE_CBITERAVX
      cbiteravx(out, pi, m, n, temp);
    #elif defined(CHOOSE_CBITERATIVE)
      cbiterative(out, pi, m, n, temp);
    #elif defined(CHOOSE_CBRECURSION)
      cbrecursion(out, 0, 1, pi, m, n, temp);
    #elif defined(CHOOSE_CBITERATIVE_UNROLLED)
      cbiterative_unrolled(out, pi, m, n, temp);
    #else
      printf("Demo called without a choice of controlbits algorithm. \n");
      exit(1);
    #endif
}

void random_example(long long m) {
    int32_t n = 1 << m;
    
    // out points to the (2m-1)n/2 control bits we will output
    unsigned char *out = malloc( (((2*m-1)*n/2)+7)/8 ); 
    int32_t *temp = malloc(sizeof(int32_t) * 2 * n);

    // Initialise pi and fill with random example
    int16_t *pi = malloc(sizeof(uint16_t) * n);
    for (int x = 0; x < n; x++) pi[x] = x;
    shuffle_int16(pi, n, 0);

    /* Store a correct copy of pi. */
    int16_t *pi_copy = malloc(sizeof(uint16_t) * n);
    for (int x = 0; x < n; x++) pi_copy[x] = pi[x];
    printf("Generated random permutation:   "); print_permutation_int16(pi_copy, n);
    
    memset(out,0,(((2*m-1)*n/2)+7)/8);
    controlbits_alg(out, pi, m, n, temp);
  
    printf("Returned Control bits:          "); print_control_bits(out, m);
    if (check_cb_correctness(pi_copy, n, out) == 1) printf("Control bits are correct. \n \n");

    free(pi);
    free(out);
}


int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Enter an argument of m (where |pi|=2^m) to choose it next time.\n");
    printf("Defaulting to m=6. \n\n");

    random_example(6);
  }
  else {
    int m = strtol(argv[1], NULL, 0);
    printf("Running cbiterative for a permutation of length 2^%d. \n\n", m);
    random_example(m);
  }
  return 0;
}