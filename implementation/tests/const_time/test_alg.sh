#!/bin/bash

# We take in arguments for sort_alg, cb_alg, m, rounds
if [ $# -ne 5 ];
  then echo "Invalid number of arguments. I expect: cb_alg, sort_alg, sort_type, m, rounds"
  exit 1
fi

CB_ALG=$1
SORT_ALG=$2
SORT_TYPE=$3
M=$4
N=$((2 ** $M))
ROUNDS=$5

# Write the director header file.
echo "#define CHOOSE_$CB_ALG
#define CHOOSE_$SORT_ALG
#define $SORT_TYPE
#define M $M
#define N $N" > director.h

cc test_case.c int8_optblocker.o int16_optblocker.o int32_optblocker.o -mavx2 -O2 -o test_case 

if valgrind -q --tool=memcheck --error-exitcode=1 --exit-on-first-error=yes ./test_case; then
  echo "$CB_ALG with $SORT_ALG on size $M is probably constant-time (meets SUPERCOP's goal-constbranch & goal-constindex)."
  rm test_case
  exit 0
else
  echo "INCORRECT: control bit algorithm $CB_ALG with $SORT_ALG on size $M."
  rm test_case
  exit 1
fi
