# Overview
## src
### Control bit algorithms
The four control bit algorithms in `src/controlbits/` are
1. `cbrecursion` - copied from libmceliece release 20250507 [1].
2. `cbiterative` - Our iterative algorithm where each control bit is placed individually.
3. `cbiterative_unrolled` - Our iterative algorithm with control bits placed in bytes.
4. `cbiteravx` - Our iterative algorithm with control bits placed as `int32_t`s. Uses AVX2.

Note implementations `1` and `2` require the array of control bits to be zero-initialised.

### Sorting algorithms
The file `sorting/sort.h` defines the APIs used for sorting by the control bit algorithms. 

The recursive algorithms call the `void int32_sort(int32_t *x, long long n)` function.

 The iterative algorithms call the `void int32_sort_interlaced(int32_t *x, int32_t m, int32_t w)` function. See `docs/interlaced_sorting_networks.md`. 

The implementations in `src/sorting/` are
1. `portable/portable4.h` - copied from djbsort version `20190516` [2]
2. `avx/ibitonic.h` - interlaced bitonic sorts - see `docs/interlaced_sorting_networks.md`. 
3. `avx/permsorts/djbsort.h` - copied from djbsort version `20190516` [2]
4. `avx/ipermsorts.h` - permuted and interlaced bitonic sorts - see `docs/permuted_sorting_networks.md`


## Benchmarks
`benchmarks` contains two Python scripts to benchmark the performance of the various algorithms and reproduce the tables in the paper.

## Tests
`tests` contains both correctness tests and constant time testing for the various combinations of control bit and sorting algorithm. The correctness tests simply try the algorithms several times and check that the control bits produced do in fact realise the permutation they were calculated from.


## Requirements
`clang`, `gcc`, `python3`, `valgrind`

## Build and run `demo.c`
There is a range of combinations of controlbit algorithm, sorting algorithm, and sorting type you can try. The options are controlled by defining macros:

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

## Refs
[1] https://lib.mceliece.org/
[2] https://sorting.cr.yp.to/changes.html