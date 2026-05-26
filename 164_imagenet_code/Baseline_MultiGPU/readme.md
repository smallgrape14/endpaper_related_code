
## 为啥会报错，当workernum=32 时候？
```sh
# 存储资源不够了吗，内存不够了 out of memory了 ？
root@node9:/home/oem/xyp/imagenet_code/Baseline_MultiGPU# ./run2.sh 
output/output_b16_w32.txt
terminate called without an active exception
Exception ignored in: <function _MultiProcessingDataLoaderIter.__del__ at 0x7f21e6868c10>
Traceback (most recent call last):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1477, in __del__
    self._shutdown_workers()
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1441, in _shutdown_workers
    w.join(timeout=_utils.MP_STATUS_CHECK_INTERVAL)
  File "/usr/lib/python3.8/multiprocessing/process.py", line 149, in join
    res = self._popen.wait(timeout)
  File "/usr/lib/python3.8/multiprocessing/popen_fork.py", line 44, in wait
    if not wait([self.sentinel], timeout):
  File "/usr/lib/python3.8/multiprocessing/connection.py", line 931, in wait
    ready = selector.select(timeout)
  File "/usr/lib/python3.8/selectors.py", line 415, in select
    fd_event_list = self._selector.poll(timeout)
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/signal_handling.py", line 67, in handler
    _error_if_any_worker_fails()
RuntimeError: DataLoader worker (pid 1353361) is killed by signal: Aborted. 
```


- 报这个错误，有的建议是​​禁用 pin_memory​​：?

若使用 GPU，设置 pin_memory=False避免提前锁定内存



### Error2 : batchsize 32 ,worker 8
```sh
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception

```
### error 3: batchsize 32 worker 16
```sh
root@node9:/home/oem/xyp/imagenet_code/Baseline_MultiGPU# ./run2.sh 
output/output_b32_w16.txt
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception
terminate called without an active exception
Exception ignored in: <function _MultiProcessingDataLoaderIter.__del__ at 0x7fe6a9609ee0>
Traceback (most recent call last):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1477, in __del__
    self._shutdown_workers()
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1441, in _shutdown_workers
    w.join(timeout=_utils.MP_STATUS_CHECK_INTERVAL)
  File "/usr/lib/python3.8/multiprocessing/process.py", line 149, in join
    res = self._popen.wait(timeout)
  File "/usr/lib/python3.8/multiprocessing/popen_fork.py", line 44, in wait
    if not wait([self.sentinel], timeout):
  File "/usr/lib/python3.8/multiprocessing/connection.py", line 931, in wait
    ready = selector.select(timeout)
  File "/usr/lib/python3.8/selectors.py", line 415, in select
    fd_event_list = self._selector.poll(timeout)
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/signal_handling.py", line 67, in handler
    _error_if_any_worker_fails()
RuntimeError: DataLoader worker (pid 1551714) is killed by signal: Aborted. 
Exception ignored in: <function _MultiProcessingDataLoaderIter.__del__ at 0x7fe629a3dee0>
Traceback (most recent call last):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1477, in __del__
    self._shutdown_workers()
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1441, in _shutdown_workers
    w.join(timeout=_utils.MP_STATUS_CHECK_INTERVAL)
  File "/usr/lib/python3.8/multiprocessing/process.py", line 149, in join
    res = self._popen.wait(timeout)
  File "/usr/lib/python3.8/multiprocessing/popen_fork.py", line 44, in wait
    if not wait([self.sentinel], timeout):
  File "/usr/lib/python3.8/multiprocessing/connection.py", line 931, in wait
    ready = selector.select(timeout)
  File "/usr/lib/python3.8/selectors.py", line 415, in select
    fd_event_list = self._selector.poll(timeout)
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/signal_handling.py", line 67, in handler
    _error_if_any_worker_fails()
RuntimeError: DataLoader worker (pid 1551713) is killed by signal: Aborted. 

```