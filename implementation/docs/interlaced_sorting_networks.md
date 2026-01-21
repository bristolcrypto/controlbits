# Interlaced Sorting Networks.

## API definition and notation.
All of the interlaced sorting network implementations conform to the following API:
```c
int32_sort_interlaced(int32_t* x, int32_t m, int32_t w)
```
Where `*x` is a pointer to the array of $2^m$ elements we are sorting, and `w` parameterises the structure in the array, i.e., for all $i < 2^m$, $x[i] \equiv i \mod 2^w$. 

## Core insight.
Consider the interlaced bitonic sort `ibitonic(m,w)` where `m` and `w` are as above. The core insight is that `ibitonic(m, w)` *looks and behaves like* `ibitonic(m-w, 0)`. For example, look at the the following plots (made with matplotlib) of the comparisons that occur in `ibitonic(5,1)`:
![](/docs/images/ibitonic(5,1).png)
and in `ibitonic(8, 4)`:
![](/docs/images/ibitonic(8,4).png)
The red comparators place the maximum value in the lower index and the blue comparators place the maximum value in the upper index. 

## Implementation strategy.

### Scalar implementation.
To create a scalar implementation of an interlaced sorting network, simply take an implementation of a standard sorting network and remove any comparators that compare elements of distance less than $2^{w}$.  

### Vectorised implementation.

#### Recursive relation.
The standard recursive construction of the bitonic sorting network also applies to the interlaced variant:

```
                      +------------------+-----------+
                      |   upside-down    |           |
+----------------+    | ibitonic(m-1, w) |   merge   |
|                |    |                  |  (m, w)   |
| ibitonic(m, w) |  = +------------------+           |
|                |    |                  |           |
+----------------+    | ibitonic(m-1, w) |           |
                      |                  |           | 
                      +------------------+-----------+
```

We fully expand the recursion for an iterative implementation. For example, the next step of the above expansion would be: 

```
                      +------------------+-----------+--------+
                      |   upside-down    |           |        |
                      | ibitonic(m-2, w) |  upside   |        |
                      |                  |   down    |        |
                      +------------------+   merge   |        |
                      |                  |  (m-1, w) |        |
+----------------+    | ibitonic(m-2, w) |           |        | 
|                |    |                  |           |        |       
| ibitonic(m, w) |  = +------------------+-----------+  merge |
|                |    |   upside-down    |           | (m, w) |
+----------------+    | ibitonic(m-2, w) |           |        | 
                      |                  |           |        | 
                      +------------------+   merge   |        | 
                      |                  |  (m-1, w) |        | 
                      | ibitonic(m-2, w) |           |        | 
                      |                  |           |        | 
                      +------------------+-----------+--------+    
```
Suppose `ibitonic(m-2, w)` is the base case of the recursion. We apply the network by iterating over the columns. We first apply all the base cases, then work rightwards through the merges. The function `ibitonic(*x, m, w)` in `src/sorting/avx/ibitonic.h` handles this logic. We explain below how `merge` is implemented. When $w\leq 2$, the base cases are `ibitonic(5, w)`. When $w \geq 3$, the base cases are `ibitonic(w+4, w)`. 


#### Merging
The merge stages are very regular. Note that `merge(w+j, w)` begins with a layer comparators of width $2^{w+j-1}$. It ends with comparators of width $2^w$. This is $j$ layers in total. We try and apply as many comparators as possible per memory load and store, to do this we use a depth-first approach (see below).

We apply at most $3$ layers per load/store. Obviously, $3$ does not necessarily divide `j`, therefore we allow for batches of size 2 and singletons. We choose the first batch to be of size 1, 2, or 3 so that the remaining batches can all be of size 3. We want the rightmost batch to be of size 3 as this covers the most complex stages of the `w < 3`.  

The code for merging is contained in `src/sorting/avx/ibitonic/ibitonic_merge.h`.

For example, `merge(8, 3)` looks as follows:
![](/docs/images/merge(8,3).png)
This diagram would correspond to a `merge_two_stages` followed by a `merge_three_stages`.

##### Depth-first application of comparators
We use a 'depth-first' application of comparators to reduce the number of memory loads and stores. This means: once we have loaded a collection of comparators, we will perform every comparison required amongst them before storing. This requires performing the comparators in a different order to which they are defined. This is okay as long as you only reorder disjoint comparators.

We can perform atleast three layers of a merge with a single memory store and load per index. To merge layers with comparator widths $2^{k-1}$, $2^{k-2}$, and $2^{k-3}$. We load in $8$ windows, parameterised by $0\leq i < 2^{k-3}$, at indices $8i+j\cdot 2^{k-3}$ for $0 \leq j < 8$. We apply the comparators of size $2^{k-1}$ by comparing windows $j=0,1,2,3$ with windows $j=4,5,6,7$ resp. We apply comparators of size $2^{k-2}$ by comparing windows $j=0,1,4,5$ with windows $j=2,3,6,7$ resp. We apply comparators of size $2^{k-3}$ by comparing windows $j=0,2,4,6$ with windows $j=1,3,5,7$ resp.

A similar approach can be done for two layers.

When we plot the comparators in the order that they are applied (left=first, right=last), our sorting network for `ibitonic(9,4)` takes the following form. The striated merge blocks on the right are due to the depth-first ordering.
![](/docs/images/ibitonic(9,4)_as_applied.png)
Note this diagram exactly corresponds to the first diagram of the recursive construction above. The big blobs on the left are the base cases `ibitonic(8, 4)`. The blue on the right is the `merge(9,4)`. 


#### Base case
The base cases of `ibitonic` are found in `src/sorting/avx/ibitonic/base_cases.h`. They are chosen again by the depth-first ordering principle. The cases $w \leq 2$ load in $64$ elements and perform as many sorts as possible on them. The $w \geq 3$ cases load in $2^{w+4}$ elements and sort them by sliding windows.
##### Case $w < 3$ 
When $w < 3$, there exist comparators whose indices are of distance $<8$. Therefore, applying these comparators with the AVX `min-max` instructions requires the use of swizzles. To minimise the number of swizzles required per comparator applied in the base cases, we load in $64$ contiguous indices into $8$ `ymm` registers and apply all the comparators of length $\leq 32$ in the depth first fashion. As the combination of swizzles is different for all three cases $w=0,1,2$, we create $3$ base-case functions.
1. `void chunk_of_64_w0(int32_t * x);`
2. `void chunk_of_64_w1(int32_t * x);`
3. `void chunk_of_64_w2(int32_t * x);`

All of these functions take as input `*x` and applies (in-place) `ibitonic(5, w)` to the lower half `x[0:31]` and `upside-down ibitonic(5, w)` to the upper `x[32:63]`. Note that the merge stages require alternating sequences of normal/upside-down base cases, therefore: whilst we *could* have applied `ibitonic(6, w)`, this would then require another three functions for the upside-down variants.

##### Case $w \geq 3$
When $w \geq 3$, we require no swizzle instructions when implementing `ibitonic(m, w)`. We use `ibitonic(w+4, w)` as the base case. Note that `ibitonic(w+i, w)` *looks and behaves like* `ibitonic(i, 0)`. We can implement `ibitonic(w+4, w)` with $2^4 = 16$ windows, sliding through positions $wj \to w(j+1)$ for $j$ in range $0$ through $15$. See the discussion on depth-first application of comparators.


