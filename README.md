# STT dynamic forest C++ library and benchmarks

This repository contains a simplified dynamic forest library based on search trees on trees. See https://github.com/berendsohn/stt-rs for a more advanced implementation of the data strutures in Rust.

The library itself is contained in `stt-cpp`.
The `dtree` directory contains David Eisenstat's dtree library (see https://www.davideisenstat.com/dtree/) and a small program based on that library, for comparison with ours.
The `stt-rs` submodule points towards our Rust implementation and is used both for comparison and to generate the benchmark data.
The `tarjan-werneck` library contains a small program based on Tarjan and Werneck's dynamic tree implementation. To properly compile it and include it in the comparisons, you need to obtain the source code and place it into `tarjan-werneck/dyntrees`.

All contents of this repository are published by me (Benjamin Aram Berendsohn) under the MIT license (see `LICENSE.txt`), *except* the contents of the `dtree/dtree-May_2014` (see copyright notices there).

## Generating data

Run
```
./generate_data.sh
```
to generate test data using the rust stt implementation.
Obviously, this requires the stt-rs submodule to be correctly checked out. The compilation is handled by the script itself.
The data generation will take a while. It can be sped up by excluding the larger tests; for this, edit the `generate_data.sh` script accordingly.

## Compiling and running the benchmark

To compile and run the benchmark, run
```
./build.sh
./benchmark_all.sh
```
The results will be printed directly to the terminal; if you want the results to be written to a file, pass it to the script, like so:
```
./benchmark_all.sh results.jsonl
```

After building, the implementations can also be tested against each other, using the generated data, with
```
./test.sh
```

## Comparing variants of the STT data structure

The implementations in `stt-cpp` each include multiple variants that can be enabled via compile flags. See the source code for more information. You can benchmark the different variants by running.
```
cd stt-rs
./bench.sh
```

This requires the generated benchmark data. It only tests one of the data files, which can be changed by editing `bench.sh`.
