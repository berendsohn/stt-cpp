#!/bin/bash

SIZES=( 1000 5000 10000 50000 100000 200000 )
BENCH_REPEAT=1
JSONL_OUTPUT_FILE=$1

if [ ! -d data ]; then
  echo "ERROR: No test data found. Please generate it first"
  exit 1
fi

if [[ $JSONL_OUTPUT_FILE ]] then
	echo "Writing results to $JSONL_OUTPUT_FILE"
	rm -f $JSONL_OUTPUT_FILE
fi

function bench {
	if [ -z "$JSONL_OUTPUT_FILE" ]; then
		echo $2
		$1 bench $BENCH_REPEAT $f | sed 's/^/  /'
		echo
	else
		echo $2...
		$1 bench --json $BENCH_REPEAT $f >> $JSONL_OUTPUT_FILE
	fi
}

for f in data/con_*.txt; do
	echo "### Input file: $f ###"
	
	n=`echo "$f" | sed -e 's/data\/con_\(.*\)_[0-9]*\.txt/\1/'`
	
	bench ./stt-cpp/bin/mtr_stt "MTR-STT C++ optimized"
	bench ./stt-cpp/bin/greedy_stt "Greedy SplayTT C++ optimized"
	bench ./stt-cpp/bin/ltp_stt "LTP SplayTT C++ optimized"
	bench ./dtree/dtree_queries "dtree link-cut"
	
	if (( n <= 50000 )); then
		if [ -f "tarjan-werneck/connectivity_st_v" ]; then
			bench ./tarjan-werneck/connectivity_st_v "Tarjan-Werneck link-cut (with empty vertex weights)"
		fi
		if [ -f "tarjan-werneck/connectivity_st_e" ]; then
			bench ./tarjan-werneck/connectivity_st_e "Tarjan-Werneck link-cut (with empty edge weights)"
		fi
		
		# Note: Rust implementations only run once each.
		if [ -z "$JSONL_OUTPUT_FILE" ]; then
			echo
			# This script prints titles itself
			./stt-rs/stt-benchmarks/target/release/bench_queries -i $f --print -r $BENCH_REPEAT link-cut stable-greedy-splay local-stable-two-pass-splay stable-move-to-root
			echo
		else
			echo "Rust implementations..."
			./stt-rs/stt-benchmarks/target/release/bench_queries -i $f --json -r $BENCH_REPEAT link-cut stable-greedy-splay local-stable-two-pass-splay stable-move-to-root >> $JSONL_OUTPUT_FILE
		fi
	fi
	
	echo
done
