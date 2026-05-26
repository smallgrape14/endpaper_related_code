// fixed_size_latency_test.cpp
#include <iostream>
#include <fstream>
#include <chrono>
#include <cuda_runtime.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include<string.h>

class FixedSizeLatencyTest {
private:
    std::string directory_path_;
    size_t file_size_;  // IOSIZE
    
public:
    struct FileResult {
        std::string filename;
        long long ssd_to_cpu_time;  // SSD→CPU延迟(微秒)
        long long cpu_to_gpu_time;  // CPU→GPU延迟(微秒)
        long long total_time;       // 总延迟(微秒)
        double bandwidth_mbps;      // 带宽(MB/s)
    };
    
    struct Statistics {
        double avg_ssd_to_cpu;      // 平均SSD→CPU延迟
        double avg_cpu_to_gpu;      // 平均CPU→GPU延迟
        double avg_total;           // 平均总延迟
        double min_total;           // 最小总延迟
        double max_total;           // 最大总延迟
        double stddev_total;        // 总延迟标准差
        double median_total;        // 总延迟中位数
        double p95_total;           // 95百分位延迟
        double p99_total;           // 99百分位延迟
        double avg_bandwidth_mbps;  // 平均带宽(MB/s)
        double min_bandwidth_mbps;  // 最小带宽(MB/s)
        double max_bandwidth_mbps;  // 最大带宽(MB/s)
    };
    
    FixedSizeLatencyTest(const std::string& directory_path, size_t file_size) 
        : directory_path_(directory_path), file_size_(file_size) {}
    
    // 获取目录下所有文件
    std::vector<std::string> get_files_in_directory() {
        std::vector<std::string> files;
        
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
            
            std::string filename = entry->d_name;
            std::string full_path = directory_path_ + "/" + filename;
            
            // 检查是否是普通文件
            struct stat stat_buf;
            if (stat(full_path.c_str(), &stat_buf) == 0 && S_ISREG(stat_buf.st_mode)) {
                // 可选：检查文件大小是否符合IOSIZE
                if (stat_buf.st_size != file_size_) {
                    std::cout << "警告: 文件 " << filename << " 大小(" << stat_buf.st_size 
                              << "字节)不等于IOSIZE(" << file_size_ << "字节)" << std::endl;
                }
                files.push_back(filename);
            }
        }
        
        closedir(dir);
        
        // 按文件名排序，确保测试顺序一致
        std::sort(files.begin(), files.end());
        
        return files;
    }
    
    // 执行延迟测试
    std::vector<FileResult> run_test() {
        std::vector<FileResult> results;
        
        // 获取文件列表
        auto files = get_files_in_directory();
        
        if (files.empty()) {
            std::cout << "目录中没有找到文件" << std::endl;
            return results;
        }
        
        std::cout << "找到 " << files.size() << " 个文件" << std::endl;
        std::cout << "IOSIZE: " << format_size(file_size_) << std::endl;
        
        // 预热：读取第一个文件但不记录
        if (!files.empty()) {
            std::cout << "执行预热..." << std::endl;
            void* gpu_buffer = allocate_gpu_buffer();
            std::string warmup_file = directory_path_ + "/" + files[0];
            measure_file_latency(warmup_file, gpu_buffer);
            cudaFree(gpu_buffer);
        }
        
        // 分配GPU显存
        void* gpu_buffer = allocate_gpu_buffer();
        
        // 测试所有文件
        std::cout << "\n开始测试所有文件..." << std::endl;
        
        for (size_t i = 0; i < files.size(); i++) {
            std::string full_path = directory_path_ + "/" + files[i];
            
            if ((i + 1) % 100 == 0 || i + 1 == files.size()) {
                std::cout << "进度: " << (i + 1) << "/" << files.size() 
                          << " (" << std::fixed << std::setprecision(1) 
                          << (i + 1) * 100.0 / files.size() << "%)" << std::endl;
            }
            
            try {
                FileResult result = measure_file_latency(full_path, gpu_buffer);
                result.filename = files[i];
                results.push_back(result);
            } catch (const std::exception& e) {
                std::cout << "文件 " << files[i] << " 测试失败: " << e.what() << std::endl;
            }
        }
        
        cudaFree(gpu_buffer);
        
        return results;
    }
    
private:
    // 分配GPU显存
    void* allocate_gpu_buffer() {
        void* gpu_buffer;
        cudaError_t alloc_status = cudaMalloc(&gpu_buffer, file_size_);
        if (alloc_status != cudaSuccess) {
            throw std::runtime_error("GPU内存分配失败: " + 
                                   std::string(cudaGetErrorString(alloc_status)));
        }
        return gpu_buffer;
    }
    
    // 测量单个文件延迟
    FileResult measure_file_latency(const std::string& full_path, void* gpu_buffer) {
        FileResult result;
        
        // 分配CPU内存
        void* cpu_buffer = malloc(file_size_);
        if (!cpu_buffer) {
            throw std::runtime_error("CPU内存分配失败");
        }
        
        auto start_total = std::chrono::high_resolution_clock::now();
        
        // 从SSD读取到CPU内存
        auto start_ssd = std::chrono::high_resolution_clock::now();
        std::ifstream file(full_path, std::ios::binary);
        if (!file) {
            free(cpu_buffer);
            throw std::runtime_error("无法打开文件: " + full_path);
        }
        
        file.read(static_cast<char*>(cpu_buffer), file_size_);
        file.close();
        auto end_ssd = std::chrono::high_resolution_clock::now();
        
        // 从CPU内存拷贝到GPU显存
        auto start_copy = std::chrono::high_resolution_clock::now();
        cudaError_t cuda_status = cudaMemcpy(gpu_buffer, cpu_buffer, file_size_, 
                                            cudaMemcpyHostToDevice);
        auto end_copy = std::chrono::high_resolution_clock::now();
        
        auto end_total = std::chrono::high_resolution_clock::now();
        
        // 计算各阶段耗时
        result.ssd_to_cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_ssd - start_ssd).count();
        result.cpu_to_gpu_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_copy - start_copy).count();
        result.total_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_total - start_total).count();
        
        // 计算带宽
        double total_time_seconds = result.total_time / 1e6;
        result.bandwidth_mbps = (file_size_ / (1024.0 * 1024.0)) / total_time_seconds;
        
        free(cpu_buffer);
        
        if (cuda_status != cudaSuccess) {
            throw std::runtime_error("CUDA内存拷贝失败");
        }
        
        return result;
    }
    
    std::string format_size(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unit_index = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024 && unit_index < 3) {
            size /= 1024;
            unit_index++;
        }
        
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_index]);
        return std::string(buffer);
    }
    
    // 计算百分位数
    double calculate_percentile(const std::vector<double>& sorted_values, double percentile) {
        if (sorted_values.empty()) return 0.0;
        
        double index = percentile * (sorted_values.size() - 1);
        int lower_index = static_cast<int>(index);
        int upper_index = lower_index + 1;
        
        if (upper_index >= sorted_values.size()) {
            return sorted_values[lower_index];
        }
        
        double fraction = index - lower_index;
        return sorted_values[lower_index] + fraction * (sorted_values[upper_index] - sorted_values[lower_index]);
    }
    
public:
    // 计算统计信息
    Statistics calculate_statistics(const std::vector<FileResult>& results) {
        Statistics stats;
        
        if (results.empty()) {
            return stats;
        }
        
        // 提取总延迟和带宽
        std::vector<double> total_times;
        std::vector<double> ssd_times;
        std::vector<double> cpu_times;
        std::vector<double> bandwidths;
        
        double sum_total = 0.0;
        double sum_ssd = 0.0;
        double sum_cpu = 0.0;
        double sum_bandwidth = 0.0;
        
        for (const auto& result : results) {
            total_times.push_back(result.total_time);
            ssd_times.push_back(result.ssd_to_cpu_time);
            cpu_times.push_back(result.cpu_to_gpu_time);
            bandwidths.push_back(result.bandwidth_mbps);
            
            sum_total += result.total_time;
            sum_ssd += result.ssd_to_cpu_time;
            sum_cpu += result.cpu_to_gpu_time;
            sum_bandwidth += result.bandwidth_mbps;
        }
        
        // 计算平均值
        stats.avg_total = sum_total / results.size();
        stats.avg_ssd_to_cpu = sum_ssd / results.size();
        stats.avg_cpu_to_gpu = sum_cpu / results.size();
        stats.avg_bandwidth_mbps = sum_bandwidth / results.size();
        
        // 计算最小值和最大值
        auto minmax_total = std::minmax_element(total_times.begin(), total_times.end());
        auto minmax_bandwidth = std::minmax_element(bandwidths.begin(), bandwidths.end());
        
        stats.min_total = *minmax_total.first;
        stats.max_total = *minmax_total.second;
        stats.min_bandwidth_mbps = *minmax_bandwidth.first;
        stats.max_bandwidth_mbps = *minmax_bandwidth.second;
        
        // 计算标准差
        double sum_sq_diff = 0.0;
        for (double time : total_times) {
            double diff = time - stats.avg_total;
            sum_sq_diff += diff * diff;
        }
        stats.stddev_total = sqrt(sum_sq_diff / results.size());
        
        // 排序用于百分位数和中位数计算
        std::sort(total_times.begin(), total_times.end());
        stats.median_total = calculate_percentile(total_times, 0.5);
        stats.p95_total = calculate_percentile(total_times, 0.95);
        stats.p99_total = calculate_percentile(total_times, 0.99);
        
        return stats;
    }
    
    // 打印统计结果
    void print_statistics(const Statistics& stats, int file_count) {
        std::cout << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "          固定大小文件延迟测试报告" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "测试配置:" << std::endl;
        std::cout << "  目录: " << directory_path_ << std::endl;
        std::cout << "  文件数量: " << file_count << std::endl;
        std::cout << "  文件大小(IOSIZE): " << format_size(file_size_) << std::endl;
        std::cout << std::endl;
        
        std::cout << "延迟统计(微秒):" << std::endl;
        std::cout << "  SSD→CPU平均延迟: " << std::fixed << std::setprecision(1) 
                  << stats.avg_ssd_to_cpu << " μs" << std::endl;
        std::cout << "  CPU→GPU平均延迟: " << stats.avg_cpu_to_gpu << " μs" << std::endl;
        std::cout << "  总平均延迟: " << stats.avg_total << " μs" << std::endl;
        std::cout << std::endl;
        
        std::cout << "总延迟分布:" << std::endl;
        std::cout << "  最小值: " << stats.min_total << " μs" << std::endl;
        std::cout << "  最大值: " << stats.max_total << " μs" << std::endl;
        std::cout << "  中位数: " << stats.median_total << " μs" << std::endl;
        std::cout << "  P95: " << stats.p95_total << " μs" << std::endl;
        std::cout << "  P99: " << stats.p99_total << " μs" << std::endl;
        std::cout << "  标准差: " << stats.stddev_total << " μs" << std::endl;
        std::cout << "  波动率: " << std::setprecision(1) 
                  << (stats.stddev_total / stats.avg_total * 100) << "%" << std::endl;
        std::cout << std::endl;
        
        std::cout << "带宽统计(MB/s):" << std::endl;
        std::cout << "  平均带宽: " << std::fixed << std::setprecision(2) 
                  << stats.avg_bandwidth_mbps << " MB/s" << std::endl;
        std::cout << "  最小带宽: " << stats.min_bandwidth_mbps << " MB/s" << std::endl;
        std::cout << "  最大带宽: " << stats.max_bandwidth_mbps << " MB/s" << std::endl;
        std::cout << std::endl;
        
        std::cout << "阶段延迟占比:" << std::endl;
        double ssd_percentage = (stats.avg_ssd_to_cpu / stats.avg_total) * 100;
        double cpu_percentage = (stats.avg_cpu_to_gpu / stats.avg_total) * 100;
        std::cout << "  SSD→CPU: " << std::fixed << std::setprecision(1) 
                  << ssd_percentage << "%" << std::endl;
        std::cout << "  CPU→GPU: " << cpu_percentage << "%" << std::endl;
        
        // 延迟范围分析
        std::cout << std::endl;
        std::cout << "延迟范围分析:" << std::endl;
        if (stats.avg_total < 1000) {
            std::cout << "  < 1ms: 优秀" << std::endl;
        } else if (stats.avg_total < 10000) {
            std::cout << "  1-10ms: 良好" << std::endl;
        } else if (stats.avg_total < 100000) {
            std::cout << "  10-100ms: 一般" << std::endl;
        } else {
            std::cout << "  > 100ms: 较慢" << std::endl;
        }
        
        std::cout << "==========================================" << std::endl;
    }
    
    // 保存详细结果到CSV
    void save_detailed_results(const std::vector<FileResult>& results, 
                               const std::string& output_file) {
        std::ofstream csv(output_file);
        if (!csv) {
            std::cout << "无法创建CSV文件: " << output_file << std::endl;
            return;
        }
        
        // 写入CSV头部
        csv << "文件名,SSD→CPU延迟(μs),CPU→GPU延迟(μs),总延迟(μs),带宽(MB/s)\n";
        
        // 写入数据
        for (const auto& result : results) {
            csv << result.filename << ","
                << result.ssd_to_cpu_time << ","
                << result.cpu_to_gpu_time << ","
                << result.total_time << ","
                << std::fixed << std::setprecision(2) << result.bandwidth_mbps << "\n";
        }
        
        csv.close();
        std::cout << "详细结果已保存到: " << output_file << std::endl;
    }
};

// 主函数
int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "固定大小文件延迟测试工具" << std::endl;
        std::cout << "用法: " << argv[0] << " <目录路径> <IOSIZE字节> <输出文件名前缀>" << std::endl;
        std::cout << "示例: " << argv[0] << " ./data 1048576 test_results" << std::endl;
        std::cout << "      测试目录./data下所有1048576字节(1MB)的文件" << std::endl;
        return 1;
    }
    
    std::string directory_path = argv[1];
    size_t iosize = std::stoul(argv[2]);
    std::string output_prefix = argv[3];
    
    std::cout << "=== 固定大小文件延迟测试 ===" << std::endl;
    std::cout << "目录: " << directory_path << std::endl;
    std::cout << "IOSIZE: " << iosize << " 字节 (" << iosize / 1024.0 << " KB)" << std::endl;
    std::cout << "输出前缀: " << output_prefix << std::endl;
    std::cout << std::endl;
    
    try {
        FixedSizeLatencyTest tester(directory_path, iosize);
        
        // 运行测试
        auto start_test = std::chrono::high_resolution_clock::now();
        auto results = tester.run_test();
        auto end_test = std::chrono::high_resolution_clock::now();
        
        double test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_test - start_test).count() / 1000.0;
        
        if (results.empty()) {
            std::cout << "没有测试结果" << std::endl;
            return 1;
        }
        
        std::cout << "\n测试完成！" << std::endl;
        std::cout << "有效测试文件数: " << results.size() << std::endl;
        std::cout << "总测试时间: " << std::fixed << std::setprecision(2) 
                  << test_duration << " 秒" << std::endl;
        
        // 计算统计信息
        auto stats = tester.calculate_statistics(results);
        
        // 打印统计结果
        tester.print_statistics(stats, results.size());
        
        // 保存详细结果
        std::string detailed_csv = output_prefix + "_detailed.csv";
        tester.save_detailed_results(results, detailed_csv);
        
        // 保存汇总统计
        std::string summary_csv = output_prefix + "_summary.csv";
        std::ofstream summary(summary_csv);
        if (summary) {
            summary << "指标,值,单位\n";
            summary << "文件数量," << results.size() << ",个\n";
            summary << "文件大小," << iosize << ",字节\n";
            summary << "SSD→CPU平均延迟," << stats.avg_ssd_to_cpu << ",微秒\n";
            summary << "CPU→GPU平均延迟," << stats.avg_cpu_to_gpu << ",微秒\n";
            summary << "总平均延迟," << stats.avg_total << ",微秒\n";
            summary << "最小总延迟," << stats.min_total << ",微秒\n";
            summary << "最大总延迟," << stats.max_total << ",微秒\n";
            summary << "中位数延迟," << stats.median_total << ",微秒\n";
            summary << "P95延迟," << stats.p95_total << ",微秒\n";
            summary << "P99延迟," << stats.p99_total << ",微秒\n";
            summary << "标准差," << stats.stddev_total << ",微秒\n";
            summary << "平均带宽," << stats.avg_bandwidth_mbps << ",MB/s\n";
            summary << "最小带宽," << stats.min_bandwidth_mbps << ",MB/s\n";
            summary << "最大带宽," << stats.max_bandwidth_mbps << ",MB/s\n";
            summary << "SSD阶段占比," << (stats.avg_ssd_to_cpu / stats.avg_total * 100) << ",%\n";
            summary << "CPU→GPU阶段占比," << (stats.avg_cpu_to_gpu / stats.avg_total * 100) << ",%\n";
            
            summary.close();
            std::cout << "汇总统计已保存到: " << summary_csv << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}