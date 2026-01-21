# Overview

## Control bit algorithms
The four control bit algorithms in `src/controlbits/` are
1. `cbrecursion` - copied from libmceliece release 20250507 [1].
2. `cbiterative` - Our iterative algorithm where each control bit is placed individually.
3. `cbiterative_unrolled` - Our iterative algorithm with control bits placed in bytes.
4. `cbiteravx` - Our iterative algorithm with control bits placed as `int32_t`s. Uses AVX2.

Note implementations `1` and `2` require the array of control bits to be zero-initialised.

## Sorting algorithms
The file `sorting/sort.h` defines the APIs used for sorting by the control bit algorithms. 

The recursive algorithms call the `void int32_sort(int32_t *x, long long n)` function.

 The iterative algorithms call the `void int32_sort_interlaced(int32_t *x, int32_t m, int32_t w)` function. See `interlaced_sorting_networks.md`. 




The implementations in `src/sorting/` are
1. `portable/portable4` - copied from djbsort version `20190516` [2]




[1] https://lib.mceliece.org/
[2] https://sorting.cr.yp.to/changes.html