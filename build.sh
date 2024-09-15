#!/bin/bash

echo "Building executables..."
(cd stt-cpp && make --silent bin/mtr_stt)
(cd stt-cpp && make --silent bin/greedy_stt)
(cd stt-cpp && make --silent bin/ltp_stt)
(cd dtree && make --silent)

(cd stt-rs && ./build_bench.sh -q)

if [ -d tarjan-werneck/dyntrees ]; then
	(cd tarjan-werneck && make connectivity_st_v --silent)
	(cd tarjan-werneck && make connectivity_st_e --silent)
else
	echo "INFO: Tarjan-Werneck dyntrees not found."
fi

echo "Done."
