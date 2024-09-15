#!/bin/bash

# Generate data for C++ benchmarks

SIZES=( 1000 5000 10000 50000 100000 500000 1000000 )
REPEAT=10
DATA_DIR=data

if [ ! -d stt-rs ]; then
  echo "ERROR: Directory stt-rs not found. Did you correctly checkout submodules?"
  exit 1
fi

cd stt-rs
echo "Building stt-rs..."
./build_bench.sh  --features stt/unsafe_node_access || exit
cd ..

GEN_BIN="./stt-rs/stt-benchmarks/target/release/generate_connectivity_queries"
if [ ! -f $GEN_BIN ]; then
  echo "ERROR: Generator executable not found. Did you correctly checkout submodules?"
  exit 1
fi

# echo "Removing old data..."
# rm -rf $DATA_DIR
mkdir -p $DATA_DIR

# For progress bar rendering
. stt-rs/util.sh

echo "Generating data"
for n in ${SIZES[@]}
do
    let q=$n*10
	echo "n = $n, q = $q"
    progress_bar_start
    for ((i=0;i<REPEAT;i++))
    do
        s=$RANDOM
        $GEN_BIN -s $s -n $n -q $q -o "$DATA_DIR/con_${n}_${i}.txt" > /dev/null
        progress_bar_tick
    done
    progress_bar_end
done
