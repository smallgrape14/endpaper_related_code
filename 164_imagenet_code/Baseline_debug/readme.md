## 0821

```sh

这run.sh 跑的是单GPU Baseline的实验
```


## 0810
```sh

root@node9:/home/oem/xyp/imagenet_code/Baseline_debug# ./run.sh
Traceback (most recent call last):
  File "train52.py", line 568, in <module>
    train_model(model, criterion, optimizer, train_loader,device,scheduler,print_freq, num_epochs=EPOCH_NUM,prefetch_factor=prefetch)
  File "train52.py", line 185, in train_model
    for i, (inputs, labels) in enumerate(train_loader):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 630, in __next__
    data = self._next_data()
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1344, in _next_data
    return self._process_data(data)
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1370, in _process_data
    data.reraise()
  File "/usr/local/lib/python3.8/dist-packages/torch/_utils.py", line 706, in reraise
    raise exception
IndexError: Caught IndexError in DataLoader worker process 0.
Original Traceback (most recent call last):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/worker.py", line 309, in _worker_loop
    data = fetcher.fetch(index)  # type: ignore[possibly-undefined]
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/fetch.py", line 52, in fetch
    data = [self.dataset[idx] for idx in possibly_batched_index]
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/_utils/fetch.py", line 52, in <listcomp>
    data = [self.dataset[idx] for idx in possibly_batched_index]
  File "train52.py", line 58, in __getitem__
    image_path = self.image_paths[idx]
IndexError: list index out of range

^CError in atexit._run_exitfuncs:
Traceback (most recent call last):
  File "/usr/local/lib/python3.8/dist-packages/torch/utils/data/dataloader.py", line 1471, in _clean_up_worker
    w.join(timeout=_utils.MP_STATUS_CHECK_INTERVAL)
  File "/usr/lib/python3.8/multiprocessing/process.py", line 149, in join
    res = self._popen.wait(timeout)
  File "/usr/lib/python3.8/multiprocessing/popen_fork.py", line 44, in wait
    if not wait([self.sentinel], timeout):
  File "/usr/lib/python3.8/multiprocessing/connection.py", line 931, in wait
    ready = selector.select(timeout)
  File "/usr/lib/python3.8/selectors.py", line 415, in select
    fd_event_list = self._selector.poll(timeout)
KeyboardInterrupt

```