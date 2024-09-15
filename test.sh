#!/bin/bash

# Compares several implementation with each other for testing purposes

if [ ! -d data ]; then
  echo "ERROR: No test data found. Please generate it first"
  exit 1
fi

mkdir -p check

function check {
	diff check/cmp0.txt check/cmp1.txt > /dev/null && echo "  Output identical" || { echo "  ERROR: Output differs"; exit 2; }
}

for f in data/con_*.txt; do
	echo "Testing file: $f"
	
	echo "MTR-STT C++ optimized"
	./stt-cpp/bin/mtr_stt compute $f > check/cmp0.txt

	echo "Greedy SplayTT C++ optimized"
	./stt-cpp/bin/greedy_stt compute $f > check/cmp1.txt
	check

	echo "LTP SplayTT C++ optimized"
	./stt-cpp/bin/ltp_stt compute $f > check/cmp1.txt
	check
	
	echo "dtree link-cut"
	./dtree/dtree_queries compute $f > check/cmp1.txt
	check
	echo
	
	if [ -f "tarjan-werneck/connectivity_st_v" ]; then
		echo "Tarjan-Werneck link-cut (with empty vertex weights)"
		./tarjan-werneck/connectivity_st_v compute $f > check/cmp1.txt
		check
		echo
	fi
	if [ -f "tarjan-werneck/connectivity_st_e" ]; then
		echo "Tarjan-Werneck link-cut (with empty edge weights)"
		./tarjan-werneck/connectivity_st_e compute $f > check/cmp1.txt
		check
		echo
	fi
	echo
done
