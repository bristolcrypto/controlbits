#!/bin/bash

interlaced_algs_to_test=(CBITERATIVE CBITERATIVE_UNROLLED CBITERAVX)
normal_sorts=(DJBSORT PORTABLE4)
interlaced_sorts=(IPERMSORT IBITONIC PORTABLE4)

# We take in arguments for M and rounds
if [ $# -ne 2 ];
  then echo "Invalid number of arguments. You must pass in m, rounds"
  exit 1
fi

M=$1
ROUNDS=$2

# Set up cryptoint optblockers
gcc -c -fwrapv ../../src/cryptoint/int8_optblocker.c
gcc -c -fwrapv ../../src/cryptoint/int16_optblocker.c
gcc -c -fwrapv ../../src/cryptoint/int32_optblocker.c

# Test recursive implementations
for normal_alg in "${normal_sorts[@]}";
do 
  ./test_alg.sh CBRECURSION $normal_alg CHOOSE_STANDARD_SORTING $M $ROUNDS
done

echo " "
# Test looping implementations
for interlaced_alg in "${interlaced_algs_to_test[@]}";
do
  for interlaced_sort in "${interlaced_sorts[@]}";
  do
    ./test_alg.sh  $interlaced_alg $interlaced_sort CHOOSE_INTERLACED_SORTING $M $ROUNDS
  done
  echo " "
done

rm int32_optblocker.o
rm int16_optblocker.o
rm int8_optblocker.o
rm director.h