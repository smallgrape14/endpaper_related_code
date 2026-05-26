import os
import argparse
import glob
import re
import shutil

def run_single_card_workflow(baseline_dir, ours_dir, draw_threads):
    """
    单卡模式工作流：运行 time, throughput, gpu_util 脚本并归档文件。
    """
    print("--- 正在执行单卡实验工作流 ---")

    # 1. 构造并执行绘图命令
    baseline_time_path = os.path.join(baseline_dir, "time.csv")
    ours_time_path = os.path.join(ours_dir, "time.csv")
    baseline_gpu_path = os.path.join(baseline_dir, "gpu_metrics.csv")
    ours_gpu_path = os.path.join(ours_dir, "gpu_metrics.csv")
    
    threads_arg = f"--draw_threads {draw_threads}" if draw_threads else ""

    commands = [
        f"python3 time.py {baseline_time_path} {ours_time_path} {threads_arg}",
        f"python3 throughput.py {baseline_time_path} {ours_time_path} {threads_arg}",
        f"python3 gpu_util.py {baseline_gpu_path} {ours_gpu_path} {threads_arg}"
    ]

    for cmd in commands:
        print(f"正在运行: {cmd}")
        os.system(cmd)
    
    # 2. 构造新目录名并归档文件
    dir_name = os.path.basename(os.path.normpath(ours_dir))
    threads_str = draw_threads.replace(",", "_") if draw_threads else ""
    
    if threads_str:
        output_dir = f"{dir_name}_single_{threads_str}"
    else:
        output_dir = f"{dir_name}_single"
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"已创建归档目录: {output_dir}")

    # 移动生成的 .png 和 .csv 文件
    files_to_move = glob.glob("*.png") + glob.glob("*.csv")
    for file in files_to_move:
        try:
            shutil.move(file, output_dir)
            print(f"已移动文件: {file}")
        except FileNotFoundError:
            print(f"警告: 未找到文件 {file}，跳过。")
    
    print(f"--- 单卡实验归档完成 {output_dir} ---")

def run_multi_card_workflow(ours_dir):
    """
    多卡模式工作流：运行 throughput_multi_node 脚本并归档文件。
    """
    print("--- 正在执行多卡实验工作流 ---")

    # 1. 执行绘图命令
    cmd = f"python3 throughput_multi_node.py {ours_dir}"
    print(f"正在运行: {cmd}")
    os.system(cmd)
    
    # 2. 查找最大节点数
    max_nodes = 0
    pattern = re.compile(r"(\d+)_servers")
    try:
        subdirs = [d for d in os.listdir(ours_dir) if os.path.isdir(os.path.join(ours_dir, d))]
        for subdir in subdirs:
            match = pattern.match(subdir)
            if match:
                nodes = int(match.group(1))
                if nodes > max_nodes:
                    max_nodes = nodes
        if max_nodes == 0:
            raise ValueError("在ours_dir下未找到任何 *_servers 格式的子目录。")
    except Exception as e:
        print(f"错误：无法确定最大节点数。请检查目录结构。详细信息: {e}")
        return

    # 3. 构造新目录名并归档文件
    dir_name = os.path.basename(os.path.normpath(ours_dir))
    output_dir = f"{dir_name}_{max_nodes}"
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"已创建归档目录: {output_dir}")

    # 移动生成的 .png 和 .csv 文件
    files_to_move = glob.glob("*.png") + glob.glob("*.csv")
    for file in files_to_move:
        try:
            shutil.move(file, output_dir)
            print(f"已移动文件: {file}")
        except FileNotFoundError:
            print(f"警告: 未找到文件 {file}，跳过。")

    print(f"--- 多卡实验归档完成 {output_dir} ---")

def main():
    """
    主函数，处理命令行参数并调用相应的工作流。
    """
    parser = argparse.ArgumentParser(
        description="自动化实验绘图和结果归档工作流。"
    )
    parser.add_argument(
        "--mode", 
        choices=["single", "multi"], 
        required=True, 
        help="选择工作流模式：'single' (单卡) 或 'multi' (多卡)。"
    )
    parser.add_argument(
        "--baseline_dir", 
        help="单卡模式下，baseline 实验结果的根目录。"
    )
    parser.add_argument(
        "--ours_dir", 
        required=True, 
        help="我们实验结果的根目录。"
    )
    parser.add_argument(
        "--draw_threads", 
        help="单卡模式下，可选的绘图线程参数，格式如 '1,4'。"
    )
    
    args = parser.parse_args()

    # 根据模式调用不同的函数
    if args.mode == "single":
        if not args.baseline_dir:
            parser.error("单卡模式下 `--baseline_dir` 参数是必须的。")
        run_single_card_workflow(args.baseline_dir, args.ours_dir, args.draw_threads)
    elif args.mode == "multi":
        run_multi_card_workflow(args.ours_dir)

if __name__ == "__main__":
    main()