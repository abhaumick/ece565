#!/bin/bash

# No Register Forwarding
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-noRegForward-1Stage/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=inorder --noRegForward --caches --stageWidth=1
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-noRegForward-1Stage/libquantum configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=inorder --noRegForward --caches --stageWidth=1
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-noRegForward-1Stage/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=inorder --noRegForward --caches --stageWidth=1


# Branch Prediction - 0% Accuracy
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-000/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=0
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-000/libquantum configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=0
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-000/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=0

# Branch Prediction - 50% Accuracy
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-050/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=50
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-050/libquantum configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=50
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-050/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=50

# Branch Prediction - 100% Accuracy
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-100/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=100
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-100/libquantum configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=100
./build/ALPHA_MESI_CMP_directory/gem5.opt -d benchmarks/InOrder-bPredAcc-100/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=inorder --caches --stageWidth=1 --bPredAccuracy=100

