# Benchmarking:
## Prerequisites
You will need to install `libcpucycles` [1] and then run `cpucycles-open` to ensure your device has permission to use `rdpmc`. 

## Set up
To reproduce the results presented in Table 1 and Table 2, please run (in this directory on a Linux machine)
1. `python3 Table_1.py` and,
2. `python3 Table_2.py` .

For obtain reproducible results, you should  
1. disable turbo boost, 
2. Either (a) disable simultaneous multithreading or (b) find a CPU core to use that doesn't do simultaneous multithreading. The script automatically tries option (b).
3. disable underclocking on this core. 

Running `python3 Table_1.py` or `python3 Table_2.py` will guide you through these steps (and shout warnings until they are completed.)

For better optimisations, please set the `microarch` variable in `Table_2.py`. It is set to use `--mtune=meteorlake` as the experiments in the paper were performed on this microarchitecture.

## Warning
The script goes searching for a CPU core that doesn't do simultaneous multithreading. On the Ultra 7 165U CPU (the machine from which the paper reports the benchmarks), the cores which don't have simultaneous multithreading are the Efficiency cores which run on a frequency which is lower than the CPU's nominal frequency. This causes a scaling error and overcounts CPU cycles when using the `rdtsc` instruction as the Time-Stamp Counter counts nominal clock cycles. We recommend using the `rdpmc` instruction. You can also disable simultaneous multithreading by setting the contents of `/sys/devices/system/cpu/smt/control` to `off`.

## Updates
10th Feb 2026: `djbsort` in the benchmark script now refers to `djbsort` version 20260127.
13th Apr 2026: Removed hardcoded use of the x86 Time-Stamp Counter in favour of using libcpucycles. The script prints out which libcpucycles counter it is using.

[1] https://cpucycles.cr.yp.to/