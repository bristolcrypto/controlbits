# Verified non-recursive calculation of Beneš networks applied to Classic McEliece
This repository accompanies the included paper and serves as an adjunct containing its artifacts.
It contains three subdirectories.

## ./specification
This contains reference Python implementations of the algorithms presented in the paper. 
Each Python function is named after its corresponding algorithm in the paper.

## ./verification
This contains a single folder, `PermNetwork`, which is a monolithic Lean project folder.
(It could have been multiple separate folders - in a sense this would be more correct for actual
development - but multiple repositories have been combined here in order to create a more coherent
artifact story).

### ./verification/PermNetwork
`PermNetwork` is a `Mathlib`-dependent `Lean` project folder. The standard `Lean` build tool, `lake`,
should be used to manage this. `lake` manages different versions of the `Lean` toolchain:
the file `lean-toolchain` and `lakefile.toml` define its toolchain and dependencies, but you
should prefer using `lake` rather than using these to manually select and deploy a particular
version of `Lean` or `Mathlib`. With an up-to-date version of `lake`, it should be sufficient
to run `lake exe cache get` which will download the appropriate `Mathlib` cache and then
`lake build` to confirm that the proofs compile. In the event
that the `Mathlib` cache is stale, it will be possible to build `Mathlib` locally (but this will be
time consuming): equally one can run `lake update` which will update all toolchains and dependencies
and download the cache for you.

On its own, a successful run of `lake build` is significant but not necessarily meaningful to
an inexperienced reader. Our recommendation is to use `VS Code` with the `Lean` plugin to
manually explore the proofs and implementations that make up our work. This will be highly
informative about the proof techniques and the methodology we have applied.

The README.md within `./verification/PermNetwork` contains more information about exactly
what can be found within the source. As a general comment: all proofs are written as best as
possible in the style of "clean" `Batteries`/`Mathlib` formalisations. In particular, we try to
prove short lemmas as much as possible rather than large monolithic theorems, building up a
so-called "proof API" around our core definitions. This aids readability, re-use, and automation
(and is just generally a desirable practice).

## ./implementation
This contains all materials relevent to the Section 5: "Implementation and evaluation". The subdirectory
`src` contains the source code, `benchmarks` contains Python scripts that reproduce Tables 1 and 2. `docs`
contains supplementary documentation explaining the implementation techniques and background on interlaced
and permuted sorting networks. `tests` contains `bash` scripts that run correctness tests, constant-time tests,
and an adaptation to `djbsort`'s `sortverif` toolkit to verify the correcness of interlaced sorting networks. 

See the subdirectories' `README`s for more information.
