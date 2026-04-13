# Interlaced sorting network verification.
## Overview
This folder contains a small extension to Daniel J. Bernstein's `sortverif` code [1] that verifies the correctness of our interlaced sorting network implementations.

## How it works
### `sortverif` overview
We first quickly overview the structure of the `sortverif` verification tools.

1. `unroll.py` takes a compiled djbsort library and symbollically executes it being applied to an array of specified size. This outputs an unrolled representation of the program. Symbolic execution is done using the `angr` tool [2].
2. `minmax.py` is a peephole optimiser. This converts the unrolled program from a convoluted sequence of machine operations into a program given by a sequence of "min" and "max" operations.
3. `decompose.py` is a verifier that takes as input the min-max program and proves that it is a sorting program. Information on the strategy used can be found in [1].

The output of `minmax.py` is a text file representing the min-max program, of which a snippet looks as follows:
```
...
v4 = in_3_32
v17 = signedmin(v5,v4)
v19 = signedmin(v5,v17)
v2 = in_1_32
v21 = signedmin(v5,v2)
v23 = signedmin(v5,v21)
v25 = signedmin(v19,v23)
out1 = v25
v26 = signedmax(v5,v3)
v28 = signedmin(v5,v26)
v29 = signedmax(v5,v7)
v31 = signedmin(v28,v29)
v32 = signedmax(v5,v1)
v34 = signedmin(v5,v32)
...
```

The tokens `in_{x}_32` denote that this value is the input from the `x`th index in the input array. The tokens `out{x}` are defined similarly for the output array.

### Adapting `sortverif` for interlaced sorting networks.
We first recall the definition of interlaced sorting networks we use. The function `int32_sort_interlaced(*x, m, w)` applies `2 ** w` disjoint sorting networks onto `2 ** m` inputs. Each sorting network is of size `2 ** (m-w)` and they are interlaced, i.e., the input and output indicies that are acted on by the `k`th sorting network, where `0 <= k < 2**w`, are all congruent to `k` modulo `2^w`.

The adaptation is super simple. We take the output of `minmax.py` and decompose it into `2**w` disjoint sorting programs. We 
1. check that these disjoint sorting programs act on the correct indices, and
2. pass them individually into `decompose.py` to verify that they are sorting programs.

Our file that does this is called `breakup.py`. There are no edits required in `unroll.py`, `minmax.py`, or `decompose.py`. The implementation provided here uses the 2019 implementation of `sortverif`.



## Running the tool
The instructions for running the tool are as follows.  
1. Create a Python virtual environment. 
  `../sorting_verification$ python3 -m venv ./sortverif2019/venv`
  `../sorting_verification$ export VENVDIR=$(pwd)/sortverif2019/venv/bin`
2. Install angr and pyparser
  `../sorting_verification$ $VENVDIR/pip3 install angr pyparsing`
3. Run the script for a given `m` and `{PORTABLE4|IBITONIC|IPERMSORT}`. Note that `IBITONIC` and `IPERMSORT` only work for `m>=6`. E.g.,
  `../sorting_verification$ controller.sh 7 IPERMSORT`

## Realistic running times.
The unrolling in the `sortverif` can take a long time to execute and be very RAM intensive. The workload decreases as `w` increases due to the reduction in comparators applied. The `w=0` case requires the most work and details for it can be found on the djbsort website. The `m=6` case takes less than a minute on a Intel Ultra 7 165U CPU. The `m=8` case takes 80 seconds. The `m=9` case takes 200 seconds.

[1] https://sorting.cr.yp.to/verif.html
[2] https://sites.cs.ucsb.edu/~vigna/publications/2016_SP_angrSoK.pdf
