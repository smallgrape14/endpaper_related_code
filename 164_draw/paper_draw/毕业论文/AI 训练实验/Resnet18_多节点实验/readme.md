# 单机单卡端到端性能测试

和之前测的baseline对比，展示 --draw_threads 1,2,4，bak --draw_threads 1,2,4,8,16
```bash
root@ubuntu-PowerEdge-R750xa:/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/fig# 

python3 draw.py --mode single --baseline_dir /doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/imagenet_Baseline/output/0906/20250907_024026 --ours_dir /doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/IPC/Train/GPUNetIO/result/0826/20250913_183205 --draw_threads 1,2,4

--- 单卡实验归档完成 20250913_183205_single_1_2_4 ---
```

###### 250909_单机单卡

! 出于展示需要，只展示 baseline worker=1,4 的图，通过 `--draw_threads` 指定(不指定，默认全画)。存放不同过滤条件的图中，包含：
- 20250908_1_4
- 20250908_1_4_8

```bash


python3 time.py Baseline_time.csv our_time.csv --draw_threads 1,4

python3 throughput.py Baseline_time.csv  our_time.csv --draw_threads 1,4

Baseline_gpu_metrics.csv our_gpu_metrics.csv --draw_threads 1,4
```

# 多计算节点拓展带宽

```bash
root@ubuntu-PowerEdge-R750xa:/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/fig# python3 draw.py --mode multi --ours_dir /doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/IPC/Train/GPUNetIO/result/0826/20250913_025111_multi
--- 多卡实验归档完成 20250913_025111_multi_5 ---
```

###### 250910_多机单卡

多计算节点拓展带宽
- 20250910_m_3
- 20250910_m_5

```bash
root@ubuntu-PowerEdge-R750xa:/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/parser# 

./parser.sh 

/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/IPC/Train/GPUNetIO/result/0826/20250910_10_multi


root@ubuntu-PowerEdge-R750xa:/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/fig# 

python3 throughput_multi_node.py 

/doca_devel/doca_2.7.0085/GPUNetIO_Agent_wi_DPU/singleGPU_Doca_GpuNetIO_Client_Access_to_AI_Training_AsyncMode_imagenetall_0827/gpunetio_rdma_client_server_write/IPC/Train/GPUNetIO/result/0826/20250910_10_multi
```

###### 250913_多机

- 脚本固定blocknum=1，能够正常跑完1-3节点的，resnet18，bs=16-256(20250912_223447_multi) => 20250912_223447_multi_3
- 164 storage 都到 192.168.3.{} 对应网口(顺序3，2，1) (20250913_011222_multi) => 20250913_011222_multi_3
	- 3的时候观察频繁 `No available send buffer`，性能劣化(256，较20250912_223447_multi)
	- 2的时候频繁 `IndexError: not enough values found during array assignment, expected 64, got 0`，导致重跑
- 多节点顺利跑完，1-3节点单卡，4-5节点用164模拟(20250913_025111_multi) => 20250913_025111_multi_5