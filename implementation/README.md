# Requirements
`clang`, `gcc`, `python3`, `valgrind`

# Build and run
There is a range of combinations of controlbit algorithm, sorting algorithm, and sorting type you will want to try. The options are controlled by defining macros:

For `demo.c`, you choose the options by definining the self-explanatory macros:
1. `CHOOSE_CBRECURSION` with
  a. `CHOOSE_PORTABLE4` and `CHOOSE_STANDARD_SORTING`,
  b. `CHOOSE_DJBSORT`   and `CHOOSE_STANDARD_SORTING`,
  c. `CHOOSE_IBITONIC`  and `CHOOSE_STANDARD_SORTING` (This gives you AVX2-enabled Bitonic sort).
2. Any of `CHOOSE_CBITERATIVE`, `CHOOSE_CBITERATIVE_UNROLLED`, `CHOOSE_CBITERAVX` with
  a. `CHOOSE_PORTABLE4` and (`CHOOSE_STANDARD_SORTING` or `CHOOSE_INTERLACED_SORTING`),
  b. `CHOOSE_DJBSORT`   and `CHOOSE_STANDARD_SORTING`,
  c. `CHOOSE_IBITONIC`  and (`CHOOSE_STANDARD_SORTING` or `CHOOSE_INTERLACED_SORTING`),
  d. `CHOOSE_IPERMSORT` and `CHOOSE_INTERLACED_SORTING`


The testing and benchmarking scripts create `director.h` files to choose these macros.