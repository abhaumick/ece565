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

## Runtime Options

warmupCount="500000000"

fastFwdCount="14750000000"
standardSwitch="500000000"

## Project Options

bipThrottle=(
                1.00000       # LRU
                0.03125       # BIP - LRU Prob = 0.03
                0.00000       # LIP
	        )

## Benchmark Options

folder="benchmarks/mcf-14B75_14B85/"

maxinsts="1000000000"


listOfBenchmarks=(
                    "mcf"
                 )

for program in "${listOfBenchmarks[@]}"; do
    echo ""
    echo ""

    echo "Running $program"

    for throttle in "${bipThrottle[@]}"; do

    	$gem5opt -d $folder$program/$throttle $pathToRunPy $options $cacheOptions -b $program --standard-switch=$standardSwitch --maxinsts=$maxinsts --fast-forward=$fastFwdCount --warmup-insts=$warmupCount --bip_throttle=$throttle

    done

done
