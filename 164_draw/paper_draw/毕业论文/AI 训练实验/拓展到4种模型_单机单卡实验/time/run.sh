## final use

python3 time_new.py ../Baseline/Baseline_time.csv ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots
python3 throughput_new_copy.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots

python3 legend.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots