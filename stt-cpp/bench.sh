#!/bin/bash

REPEAT=10
INPUT=../data/con_100000_0.txt

for i in {0..6}; do
  echo "++ MTR VARIANT $i ++"
  make --silent bin/mtr_stt_var$i && ./bin/mtr_stt_var$i bench "$@" $REPEAT $INPUT || echo "Error in build or execution"
  echo
done

for i in {0..3}; do
  echo "++ Greedy VARIANT $i ++"
  make --silent bin/greedy_stt_var$i && ./bin/greedy_stt_var$i bench "$@" $REPEAT $INPUT || echo "Error in build or execution"
  echo
done

for i in {0..9}; do
  echo "++ L2P VARIANT $i ++"
  make --silent bin/ltp_stt_var$i && ./bin/ltp_stt_var$i bench "$@" $REPEAT $INPUT || echo "Error in build or execution"
  echo
done
