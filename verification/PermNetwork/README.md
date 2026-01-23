# PermNetwork

This library of Lean results consists of three separate pieces of work that are presented together
here.

## ComparatorNetwork
This work is in its relatively early stages: we seek to model comparator networks in Lean and
proof results about their sorting properties. Of particular note is
`permOn_cSwaps_map_of_monotone_cSwaps` in `Basic.lean`, which essentially states the following:

  Suppose we have a sorting network `x` such that when the tuple `f` is fed into the network we get
  the tuple `g`, if we permute the channels of the sorting network by a permutation `pi`, then if
  we insert the tuple `f` into the resulting comparator network, and then permute the output on the
  right by `pi`, we get `g` - in other words, permuting the channels produces a network that does
  not sort, but does sort up to permutation.

## CBRecursive
This work is an extension of Bernstein's formalisation of his control bits formulae. In particular
in `Controlbits.lean`, one can find versions of all of Bernstein's proofs. We go somewhat further
however: `toPerm_leftInverse` in the same file essentially shows that the interpretation of the
control bits as a permutation is the left inverse to the calculation of a permutation's control bits.

The implementation of the control bits calculation here is not optimal: in particular due to
decisions made in implementation, the calculation of the cycle minima is perilously inefficient.
It is correct in terms of values, but should not be used as a model for production code.

## CBIterative
This work presents the Robson-Kelly iterative variation on Bernstein's control bits formulae.
The best way of working through this is to start with `Controlbits.lean` and trace backwards
to see the way in which the formalisation is built up. Note that we do not do swaps in an
oblivious manner in this implementation, and it does lack our optimisations of the control bits
calculation using interlaced sorting networks: indeed we do not use the decorate-sort-undecorate
paradigm at all here.

As such this shows that the iterative formulae are correct, but does
not constitute a formalised proof of our algorithms (though this is a goal in future, and
is partially the aim of the work begun in `ComparatorNetwork`).