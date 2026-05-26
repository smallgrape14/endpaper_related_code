// traditional_cpu_pipeline_directory.cpp
#include <iostream>
#include <fstream>
#include <chrono>
#include <cuda_runtime.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <algorithm>

class TraditionalCPUPipelineDirectory {
private:
    std::string directory_path_;
    
public:
    TraditionalCPUPipelineDirectory(const std::string& directory_path) 
        : directory_path_(directory_path) {}
    
    struct FileInfo {
        std::string filename;
        size_t file_size;
    };
    
    struct TimingResults {
        std::string filename;
        long long ssd_to_cpu_time;  // SSD到CPU内存耗时(微秒)
        long long cpu_to_gpu_time;  // CPU到GPU拷贝耗时(微秒)
        long long total_time;       // 总耗时(微秒)
    };
    
    // 获取目录下所有文件
    std::vector<FileInfo> get_files_in_directory() {
        std::vector<FileInfo> files;
        
        DIR* dir = opendir(directory_path_.c_str());
        if (!dir) {
            throw std::runtime_error("无法打开目录: " + directory_path_);
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // 跳过.和..目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            std::string full_path = directory_path_ + "/" + entry->d_name;
            
            // 检查是否是普通文件
            struct stat stat_buf;
            if (stat(full_path.c_str(), &stat_buf) == 0 && S_ISREG(stat_buf.st_mode)) {
                FileInfo info;
                info.filename = entry->d_name;
                info.file_size = stat_buf.st_size;
                files.push_back(info);
            }
        }
        
        closedir(dir);
        
        // 按文件名排序，确保测试顺序一致
        std::sort(files.begin(), files.end(), 
                 [](const FileInfo& a, const FileInfo& b) {
                     return a.filename < b.filename;
                 });
        
        return files;
    }
    
    // 处理单个文件
    TimingResults process_single_file(const std::string& filename, size_t file_size, void* gpu_buffer) {
        TimingResults results;
        results.filename = filename;
        
        std::string full_path = directory_path_ + "/" + filename;
        
        // 1. 分配CPU内存
        void* cpu_buffer = malloc(file_size);
        if (!cpu_buffer) {
            throw std::runtime_error("CPU内存分配失败: " + filename);
        }
        
        auto start_total = std::chrono::high_resolution_clock::now();
        
        // 2. 从SSD读取到CPU内存
        auto start_ssd = std::chrono::high_resolution_clock::now();
        std::ifstream file(full_path, std::ios::binary);
        if (!file) {
            free(cpu_buffer);
            throw std::runtime_error("无法打开文件: " + full_path);
        }
        
        file.read(static_cast<char*>(cpu_buffer), file_size);
        file.close();
        auto end_ssd = std::chrono::high_resolution_clock::now();
        
        // 3. 从CPU内存拷贝到GPU显存
        auto start_copy = std::chrono::high_resolution_clock::now();
        cudaError_t cuda_status = cudaMemcpy(gpu_buffer, cpu_buffer, file_size, 
                                            cudaMemcpyHostToDevice);
        auto end_copy = std::chrono::high_resolution_clock::now();
        
        auto end_total = std::chrono::high_resolution_clock::now();
        
        // 计算各阶段耗时
        results.ssd_to_cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_ssd - start_ssd).count();
        results.cpu_to_gpu_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_copy - start_copy).count();
        results.total_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_total - start_total).count();
        
        free(cpu_buffer);
        
        if (cuda_status != cudaSuccess) {
            throw std::runtime_error("CUDA内存拷贝失败: " + 
                                   std::string(cudaGetErrorString(cuda_status)) + 
                                   " 文件: " + filename);
        }
        
        return results;
    }
    
    // 处理目录下所有文件
    std::vector<TimingResults> process_all_files(void* gpu_buffer, size_t max_buffer_size) {
        std::vector<TimingResults> all_results;
        
        // 获取目录下所有文件
        auto files = get_files_in_directory();
        
        if (files.empty()) {
            std::cout << "目录中没有找到文件" << std::endl;
            return all_results;
        }
        
        std::cout << "找到 " << files.size() << " 个文件:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  " << file.filename << " (" << file.file_size / 1024 << " KB)" << std::endl;
        }
        std::cout << std::endl;
        
        // 处理每个文件
        for (size_t i = 0; i < files.size(); i++) {
            const auto& file = files[i];
            
            // 检查文件大小是否超过分配的GPU缓冲区
            if (file.file_size > max_buffer_size) {
                std::cout << "警告: 文件 " << file.filename << " 大小(" << file.file_size 
                          << "字节)超过GPU缓冲区大小(" << max_buffer_size << "字节)，跳过" << std::endl;
                continue;
            }
            
            std::cout << "处理文件 " << (i+1) << "/" << files.size() 
                      << ": " << file.filename << " (" 
                      << file.file_size / 1024 << " KB)" << std::endl;
            
            try {
                TimingResults result = process_single_file(file.filename, file.file_size, gpu_buffer);
                all_results.push_back(result);
                
                // 打印单个文件结果
                print_single_file_result(result, i+1);
                
            } catch (const std::exception& e) {
                std::cout << "处理文件失败: " << e.what() << std::endl;
            }
        }
        
        return all_results;
    }
    
    void print_single_file_result(const TimingResults& result, int file_index) {
        std::cout << "  文件 " << file_index << " - " << result.filename << ":" << std::endl;
        std::cout << "    SSD→CPU: " << result.ssd_to_cpu_time << " μs" << std::endl;
        std::cout << "    CPU→GPU: " << result.cpu_to_gpu_time << " μs" << std::endl;
        std::cout << "    总时间: " << result.total_time << " μs" << std::endl;
        std::cout << "    拷贝占比: " << std::fixed 
                  << (static_cast<double>(result.cpu_to_gpu_time) / result.total_time * 100) 
                  << "%" << std::endl;
    }
    
    void print_summary(const std::vector<TimingResults>& results) {
        if (results.empty()) {
            std::cout << "没有可用的测试结果" << std::endl;
            return;
        }
        
        std::cout << "\n=== 测试结果汇总 ===" << std::endl;
        std::cout << "处理的文件数量: " << results.size() << std::endl;
        
        // 计算统计数据
        long long total_time_sum = 0;
        long long ssd_time_sum = 0;
        long long copy_time_sum = 0;
        size_t total_bytes = 0;
        
        // 找出最快和最慢的文件
        long long min_time = std::numeric_limits<long long>::max();
        long long max_time = 0;
        std::string fastest_file, slowest_file;
        
        for (const auto& result : results) {
            total_time_sum += result.total_time;
            ssd_time_sum += result.ssd_to_cpu_time;
            copy_time_sum += result.cpu_to_gpu_time;
            
            if (result.total_time < min_time) {
                min_time = result.total_time;
                fastest_file = result.filename;
            }
            if (result.total_time > max_time) {
                max_time = result.total_time;
                slowest_file = result.filename;
            }
        }
        
        double avg_total_time = static_cast<double>(total_time_sum) / results.size();
        double avg_ssd_time = static_cast<double>(ssd_time_sum) / results.size();
        double avg_copy_time = static_cast<double>(copy_time_sum) / results.size();
        double copy_percentage = (static_cast<double>(copy_time_sum) / total_time_sum) * 100;
        
        std::cout << "\n平均性能统计:" << std::endl;
        std::cout << "  平均SSD→CPU时间: " << avg_ssd_time << " μs" << std::endl;
        std::cout << "  平均CPU→GPU时间: " << avg_copy_time << " μs" << std::endl;
        std::cout << "  平均总时间: " << avg_total_time << " μs" << std::endl;
        std::cout << "  平均拷贝占比: " << std::fixed  
                  << copy_percentage << "%" << std::endl;
        
        std::cout << "\n性能极值:" << std::endl;
        std::cout << "  最快文件: " << fastest_file << " (" << min_time << " μs)" << std::endl;
        std::cout << "  最慢文件: " << slowest_file << " (" << max_time << " μs)" << std::endl;
        std::cout << "  波动范围: " << (max_time - min_time) << " μs (" 
                  << std::fixed 
                  << (static_cast<double>(max_time - min_time) / min_time * 100) 
                  << "% 差异)" << std::endl;
        
        // 吞吐量计算
        double total_time_seconds = static_cast<double>(total_time_sum) / 1e6; // 转换为秒
        // 假设每个文件平均1MB用于估算吞吐量
        double estimated_throughput_mb = (results.size() * 1.0) / total_time_seconds; // MB/s
        
        std::cout << "\n估算吞吐量:" << std::endl;
        std::cout << "  总时间: " << total_time_seconds << " 秒" << std::endl;
        std::cout << "  估算吞吐量: " << estimated_throughput_mb << " MB/s" << std::endl;
    }
};

// 主函数
int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cout << "用法: " << argv[0] << " <目录路径> [最大GPU缓冲区大小MB]" << std::endl;
        std::cout << "示例: " << argv[0] << " ./data 256" << std::endl;
        std::cout << "说明: 最大GPU缓冲区大小默认为1024MB" << std::endl;
        return 1;
    }
    
    std::string directory_path = argv[1];
    size_t max_buffer_size_mb = (argc == 3) ? std::stoul(argv[2]) : 1024;
    size_t max_buffer_size = max_buffer_size_mb * 1024 * 1024;
    
    std::cout << "=== 传统CPU中转方案测试（目录模式）===" << std::endl;
    std::cout << "目录: " << directory_path << std::endl;
    std::cout << "GPU缓冲区大小: " << max_buffer_size_mb << " MB" << std::endl;
    
    // 检查目录存在性
    DIR* dir = opendir(directory_path.c_str());
    if (!dir) {
        std::cout << "错误: 目录不存在或无法访问" << std::endl;
        return 1;
    }
    closedir(dir);
    
    // 分配GPU显存
    void* gpu_buffer;
    cudaError_t alloc_status = cudaMalloc(&gpu_buffer, max_buffer_size);
    if (alloc_status != cudaSuccess) {
        std::cout << "GPU内存分配失败: " << cudaGetErrorString(alloc_status) << std::endl;
        return 1;
    }
    
    std::cout << "GPU内存分配成功: " << max_buffer_size_mb << " MB" << std::endl;
    
    TraditionalCPUPipelineDirectory pipeline(directory_path);
    
    // 运行多次测试
    int num_iterations = 3;  // 可以调整迭代次数
    std::vector<std::vector<TimingResults>> all_iteration_results;
    
    std::cout << "\n开始性能测试，迭代次数: " << num_iterations << std::endl;
    
    for (int iter = 0; iter < num_iterations; iter++) {
        std::cout << "\n--- 迭代 " << (iter + 1) << "/" << num_iterations << " ---" << std::endl;
        
        try {
            auto results = pipeline.process_all_files(gpu_buffer, max_buffer_size);
            all_iteration_results.push_back(results);
            
            if (iter == 0) {
                std::cout << "第一次迭代完成（预热）" << std::endl;
            } else {
                pipeline.print_summary(results);
            }
        } catch (const std::exception& e) {
            std::cout << "迭代 " << iter << " 失败: " << e.what() << std::endl;
        }
    }
    
    // 如果有多于一次的有效迭代，计算平均性能
    if (all_iteration_results.size() > 1) {
        std::cout << "\n=== 跨迭代平均性能 ===" << std::endl;
        
        // 计算平均时间（跳过第一次迭代作为预热）
        long long total_avg_time = 0;
        long long total_avg_ssd_time = 0;
        long long total_avg_copy_time = 0;
        
        for (size_t i = 1; i < all_iteration_results.size(); i++) {
            const auto& results = all_iteration_results[i];
            
            long long iter_total_time = 0;
            long long iter_ssd_time = 0;
            long long iter_copy_time = 0;
            
            for (const auto& result : results) {
                iter_total_time += result.total_time;
                iter_ssd_time += result.ssd_to_cpu_time;
                iter_copy_time += result.cpu_to_gpu_time;
            }
            
            total_avg_time += iter_total_time;
            total_avg_ssd_time += iter_ssd_time;
            total_avg_copy_time += iter_copy_time;
        }
        
        int valid_iterations = all_iteration_results.size() - 1;
        int total_files_processed = 0;
        for (size_t i = 1; i < all_iteration_results.size(); i++) {
            total_files_processed += all_iteration_results[i].size();
        }
        
        if (total_files_processed > 0) {
            double avg_total_time = static_cast<double>(total_avg_time) / total_files_processed;
            double avg_ssd_time = static_cast<double>(total_avg_ssd_time) / total_files_processed;
            double avg_copy_time = static_cast<double>(total_avg_copy_time) / total_files_processed;
            
            std::cout << "平均每个文件的性能:" << std::endl;
            std::cout << "  SSD→CPU: " << avg_ssd_time << " μs" << std::endl;
            std::cout << "  CPU→GPU: " << avg_copy_time << " μs" << std::endl;
            std::cout << "  总时间: " << avg_total_time << " μs" << std::endl;
            std::cout << "  拷贝占比: " << std::fixed << 
                       (avg_copy_time / avg_total_time * 100) << "%" << std::endl;
        }
    }
    
    cudaFree(gpu_buffer);
    std::cout << "\n测试完成！" << std::endl;
    
    return 0;
}