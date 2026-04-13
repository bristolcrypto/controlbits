#include <stdint.h>
#include "options.h"
#include "../../src/sorting/sort.h"



#define N (1 << M)
int32_t x[N];

int main() {
  int32_sort_interlaced(x, M, W);
  return 0;
}