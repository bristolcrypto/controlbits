#!/bin/bash

# Check script is run from correct directory.
if [ $( basename $(pwd) ) != "sorting_verification" ]; then
	echo "SCRIPT CALLED FROM WRONG DIRECTORY"
	exit 1
fi

# Basic check that VENVDIR was made and points to some "bin"
if [ $( basename $VENVDIR ) != "bin" ]; then
	echo "You need to export the VENVDIR variable"
	exit 1
fi

# We take in arguments m sort_alg
if [ $# -ne 2 ];
  then echo "Invalid number of arguments. I expect: m sortalg"
  exit 1
fi

# Inputs and sanitisation
m=$1
sort_alg_inp=$2
sort_alg=${sort_alg_inp^^}
n=$(( 1<<$m ))

if [ "$sort_alg" != "IPERMSORT" ] && [ "$sort_alg" != "PORTABLE4" ] && [ "$sort_alg" != "IBITONIC" ]; then
	echo "Invalid sorting algorithm name!"
	exit 1
fi

mkdir tmp

for w in $(seq 0 $(( $m - 2 )) ); do
	two_to_the_w=$(( 1<<$w ))

	# COMPILE SORTING FUNCTION

	echo "#define M $m
#define W $w

#define CHOOSE_CBITERATIVE
#define CHOOSE_INTERLACED_SORTING
#define CHOOSE_$sort_alg" > ./options.h

	cc driver.c -O3 -mavx2 -o binary

	# Run 
	echo "Verifying interlaced sorting networks for case (m, w)=($m, $w)"
	echo "  Starting unroll at $(date)"
	$VENVDIR/python3 ./sortverif2019/unroll.py x $n binary > ./tmp/unroll 2> /dev/null
	echo "  Unroll complete at $(date)"

	$VENVDIR/python3 ./sortverif2019/minmax.py $n < ./tmp/unroll > ./tmp/minmax
	$VENVDIR/python3 ./sortverif2019/breakup.py $n int32_sort_x < ./tmp/minmax 

	rm ./tmp/minmax
	rm ./tmp/unroll

	# Check that breakup.py produced the correct number of sorting programs to verify.
	num_progs=$(eval "ls ./tmp/ | wc -l")
	if [ "$num_progs" != "$two_to_the_w" ]; then
		echo "Decomposition into disjoint sorting networks for decompose.py failed."
		exit 1
	fi

	for sortprog in ./tmp/*; do
		if ! $VENVDIR/python3 ./sortverif2019/decompose.py $((n / two_to_the_w)) int32_sort_x < $sortprog ; then
			echo "ERROR on $sortprog!"
			echo "$((n / two_to_the_w))"
			exit 1
		fi

		rm $sortprog
	done
	rm binary

	echo "Completed verification for case (m, w)=($m, $w)."

done
echo "Verified $2 is an interlaced sorting network for m=$m over all values of w passed into cbiterative."
rmdir tmp