#!/bin/bash
python3 time_init.py ../Baseline/Baseline_time.csv ../Our/our_time.csv --draw_threads 1,4
python3 time.py ../Baseline/Baseline_time.csv ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots


python3 throughput.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4

python3 throughput_new.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots
python3 throughput_new_copy.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots

python3 gpu_util.py ../Baseline/Baseline_gpu_metrics.csv ../Our/our_gpu_metrics.csv --draw_threads 1,4


## final use

python3 time_new.py ../Baseline/Baseline_time.csv ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots
python3 throughput_new_copy.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots
