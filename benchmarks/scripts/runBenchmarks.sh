#!/bin/bash

## GEM5 options

gem5opt="./build/ALPHA_MESI_CMP_directory/gem5.opt"
pathToRunPy="configs/spec2k6/run.py"

## CPU Options

options="--cpu-type=detailed --caches --l2cache"

## Cache Options

#   --l1d_size=L1D_SIZE   
#   --l1i_size=L1I_SIZE   
#   --l2_size=L2_SIZE     
#   --l3_size=L3_SIZE     
#   --l1d_assoc=L1D_ASSOC
#   --l1i_assoc=L1I_ASSOC
#   --l2_assoc=L2_ASSOC   
#   --l3_assoc=L3_ASSOC   

cacheOptions="--l1d_size=16kB --l1i_size=16kB --l2_size=1MB --l2_assoc=16"

## Benchmark Options

folder="benchmarks/default/"

maxinsts="10000000"


listOfBenchmarks=(
                    # "mcf"
                    # "bzip2"
                    # "specrand_i"
                    # "specrand_f"
                    "perlbench"
                    "bzip2"
                    "gcc"
                    "bwaves"
                    "gamess"
                    "mcf"
                    "milc"
                    "zeusmp"
                    "gromacs"
                    "cactusADM"
                    "leslie3d"
                    "namd"
                    "gobmk"
                    "dealII"
                    "soplex"
                    "povray"
                    "calculix"
                    "hmmer"
                    "sjeng"
                    "GemsFDTD"
                    "libquantum"
                    "h264ref"
                    "tonto"
                    "lbm"
                    "omnetpp"
                    "astar"
                    "wrf"
                    "sphinx3"
                    "xalancbmk"
                    "specrand_i"
                    "specrand_f"
                 )

for program in "${listOfBenchmarks[@]}"; do
    echo ""
    echo ""

    echo "Running $program"

    $gem5opt -d $folder$program $pathToRunPy $options $cacheOptions -b $program --maxinsts=$maxinsts

done
