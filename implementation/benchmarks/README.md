
# Benchmarking:
## Set up
To reproduce the results presented in Table 1 and Table 2, please run (on a Linux machine)
1. `python3 Table_1.py` and,
2. `python3 Table_2.py` .

For obtain reproducible results, you should  
1. disable turbo boost, 
2. find a single CPU core to use that doesn't to simultaneous multithreading
3. disable underclocking on this core. 

Running `python3 Table_1.py` or `python3 Table_2.py` will guide you through these steps (and shout warnings until they are completed.)

For better optimisations, please set the `microarch` variable in `Table_2.py`. It is set to use `--mtune=meteorlake` as the experiments in the paper were performed on this microarchitecture.

## Updates
10th Feb 2026: `djbsort` in the benchmark script now refers to `djbsort` version 20260127.