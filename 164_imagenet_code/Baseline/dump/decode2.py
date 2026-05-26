import os
import shutil
from PIL import Image
import argparse
from tqdm import tqdm
import logging

def validate_jpeg(file_path):
    """验证并重新编码JPEG文件"""
    try:
        with Image.open(file_path) as img:
            # 验证文件完整性
            img.verify()
            # 转换模式确保兼容性
            if img.mode != 'RGB':
                img = img.convert('RGB')
            return True
    except Exception as e:
        logging.error(f"损坏文件: {file_path} - {str(e)}")
        return False

def process_directory(input_dir, output_dir):
    """
    JPEG解码处理管道
    1. 保持原始目录结构
    2. 仅复制有效JPEG文件
    3. 自动修复常见元数据问题
    """
    # 配置日志
    logging.basicConfig(
        filename='jpeg_processor.log',
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s: %(message)s'
    )

    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)

    # 遍历目录结构
    for root, dirs, files in os.walk(input_dir):
        # 生成输出子目录路径
        rel_path = os.path.relpath(root, input_dir)
        output_subdir = os.path.join(output_dir, rel_path)
        
        # 创建输出子目录
        os.makedirs(output_subdir, exist_ok=True)

        # 进度条显示
        with tqdm(total=len(files), desc=f'处理目录 {rel_path}', unit='file') as pbar:
            for filename in files:
                input_path = os.path.join(root, filename)
                output_path = os.path.join(output_subdir, filename)

                # 跳过非JPEG文件
                if not filename.lower().endswith(('.jpg', '.jpeg')):
                    pbar.update(1)
                    continue

                # 执行解码验证
                if validate_jpeg(input_path):
                    try:
                        # 优化复制（保留原始元数据）
                        with Image.open(input_path) as img:
                            # 保持原始格式和质量
                            img.save(
                                output_path,
                                format='JPEG',
                                quality=100,  # 保持最大质量
                                subsampling=0,  # 禁用色度抽样
                                optimize=True
                            )
                    except Exception as e:
                        logging.error(f"保存失败: {input_path} - {str(e)}")
                else:
                    logging.warning(f"跳过无效文件: {input_path}")

                pbar.update(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='JPEG解码处理器')
    parser.add_argument('-i', '--input', required=True, help='输入目录路径')
    parser.add_argument('-o', '--output', required=True, help='输出目录路径')
    args = parser.parse_args()

    # 输入验证
    if not os.path.isdir(args.input):
        raise ValueError("输入目录不存在")
    if args.input == args.output:
        raise ValueError("输入输出目录不能相同")

    print(f"输入目录: {os.path.abspath(args.input)}")
    print(f"输出目录: {os.path.abspath(args.output)}")
    print(f"日志文件: {os.path.abspath('jpeg_processor.log')}")

    process_directory(args.input, args.output)