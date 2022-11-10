#!/bin/bash

cd build/3d_tree_ex
perf record -F max -g -- ./3d_tree_ex

# perf script | ./stackcollapse-perf.pl --all |./flamegraph.pl > perf_3d_plant_ex.svg