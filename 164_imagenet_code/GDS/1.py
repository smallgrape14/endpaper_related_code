@pipeline_def(batch_size=batch_size, num_threads=3, device_id=0)
def pipe_gds():
    data = fn.readers.numpy(device="gpu", file_root=data_dir, files=files)
    return data


p = pipe_gds()
p.build()
pipe_out = p.run()

# as_cpu() to copy the data back to CPU memory
data_gds = pipe_out[0].as_cpu().as_array()
print(data_gds.shape)
plot_batch(data_gds)