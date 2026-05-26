实验：
测试GPU 利用率和 SM占有率，SM activity三种指标



python3 gpu_util.py ../Baseline/Baseline_gpu_metrics.csv ../Our/our_gpu_metrics.csv --draw_threads 1,4 --output_dir my_plots


## final use

python3 time_new.py ../Baseline/Baseline_time.csv ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots
python3 throughput_new_copy.py ../Baseline/Baseline_time.csv  ../Our/our_time.csv --draw_threads 1,4 --output_dir my_plots

