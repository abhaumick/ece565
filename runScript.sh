#!/bin/bash

./build/ALPHA_MESI_CMP_directory//gem5.opt -d benchmarks configs/spec2k6/run.py --cpu-type=detailed --caches -b bzip2 --maxinsts=250000
