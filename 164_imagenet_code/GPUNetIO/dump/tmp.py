#预处理全在GPU上完成，默认数据已经在GPU显存中
# 先将数据加载到内存（网页4推荐方案）
with open("data.bin", "rb") as f:
    raw_data = f.read()

# 通过external_source输入GPU管道
@pipeline_def(batch_size=16, device_id=0)
def gpu_only_pipeline():
    data = fn.external_source(
        source=lambda: np.frombuffer(raw_data, dtype=np.uint8),
        device="gpu"  # 直接传入GPU显存
    )
    images = fn.decoders.image(data, device="gpu")
    return images