#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <map>
#include <algorithm>
// #define US_OPEN 0 //1表示开启 us 计数
using namespace std;
std::mutex cout_mutex; // 用于线程安全的输出
std::mutex file_map_mutex; // 用于线程安全的文件描述符访问

// 读取目录中的文件
std::vector<std::string> get_files_in_directory(const std::string& directory_path) {
    std::vector<std::string> files;
    DIR* dir = opendir(directory_path.c_str());
    if (!dir) {
        std::cerr << "无法打开目录: " << directory_path << std::endl;
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        files.push_back(entry->d_name);
    }

    closedir(dir);
    return files;
}

// 预先打开所有文件并返回文件描述符的映射
std::map<std::string, int> open_all_files(const std::string& directory_path, const std::vector<std::string>& files) {
    std::map<std::string, int> file_descriptors;
    std::string full_path;

    for (const auto& file : files) {
        full_path = directory_path + "/" + file;
        int fd = open(full_path.c_str(), O_RDONLY);
        if (fd < 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "无法打开文件: " << full_path << std::endl;
        } else {
            std::lock_guard<std::mutex> lock(file_map_mutex);
            file_descriptors[file] = fd;
        }
    }

    return file_descriptors;
}

// 获取每个文件的大小信息
std::map<std::string, off_t> get_file_sizes(const std::map<std::string, int>& file_descriptors) {
    std::map<std::string, off_t> file_sizes;
    struct stat file_stat;

    for (const auto& entry : file_descriptors) {
        const std::string& file_name = entry.first;
        int fd = entry.second;

        if (fstat(fd, &file_stat) == 0) {
            std::lock_guard<std::mutex> lock(file_map_mutex);
            file_sizes[file_name] = file_stat.st_size;
        } else {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "无法获取文件大小: " << file_name << std::endl;
        }
    }

    return file_sizes;
}

// 每个线程处理的函数
void process_files(const std::string& directory_path, const std::map<std::string, int>& file_descriptors, 
                   const std::map<std::string, off_t>& file_sizes, int thread_id, int start, int end,size_t iosize) {
    std::string full_path;

    for (int i = start; i < end; ++i) {
        auto it = file_descriptors.begin();
        std::advance(it, i);
        const std::string& file_name = it->first;
        int fd = it->second;

        // 记录单个文件的读取开始时间
        // auto file_start_time = std::chrono::high_resolution_clock::now();
        size_t file_size=iosize;
        if(iosize==0)
        {
            // 获取文件大小
            auto file_size_it = file_sizes.find(file_name);
            if (file_size_it == file_sizes.end()) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "无法找到文件大小: " << file_name << std::endl;
                continue;
            }
            file_size = file_size_it->second;
        }

        


        // 分配缓冲区
        char* buffer = new char[file_size];

        // 重置文件指针到开头
        lseek(fd, 0, SEEK_SET);

        // 读取整个文件
        ssize_t bytes_read = read(fd, buffer, file_size);
        if (bytes_read == -1) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "线程 " << thread_id << " 读取文件失败: " << file_name << std::endl;
            delete[] buffer;
            continue;
        }

        // 记录单个文件的读取结束时间
        // auto file_end_time = std::chrono::high_resolution_clock::now();
        // auto file_time = std::chrono::duration_cast<std::chrono::milliseconds>(file_end_time - file_start_time).count();
        // auto file_time_us = std::chrono::duration_cast<std::chrono::microseconds>(file_end_time - file_start_time).count();

        // std::lock_guard<std::mutex> lock(cout_mutex);
        // std::cout << "线程 " << thread_id << " 处理文件: " << file_name 
        //           << " (大小: " << file_size << " 字节, 读取时间: " << file_time_us << " us)" 
        //           << std::endl;

        delete[] buffer;
    }
}

// 关闭所有文件描述符
void close_all_files(const std::map<std::string, int>& file_descriptors) {
    for (const auto& entry : file_descriptors) {
        close(entry.second);
    }
}
// 将字符串IOSIZE转换为字节数
size_t parseIOSize(const string& ioSizeStr) {
    string sizeStr = ioSizeStr;
    transform(sizeStr.begin(), sizeStr.end(), sizeStr.begin(), ::toupper);
    
    size_t multiplier = 1;
    if (sizeStr.find("K") != string::npos) {
        multiplier = 1024;
        sizeStr.erase(sizeStr.find("K"), 1);
    } else if (sizeStr.find("M") != string::npos) {
        multiplier = 1024 * 1024;
        sizeStr.erase(sizeStr.find("M"), 1);
    } else if (sizeStr.find("G") != string::npos) {
        multiplier = 1024 * 1024 * 1024;
        sizeStr.erase(sizeStr.find("G"), 1);
    }
    
    try {
        size_t baseSize = stoul(sizeStr);
        return baseSize * multiplier;
    } catch (const exception& e) {
        cerr << "错误的IOSIZE格式: " << ioSizeStr << endl;
        cerr << "支持格式: 数字+K/M/G (如: 4K, 1M, 64K等)" << endl;
        exit(1);
    }
}
int main(int argc, char* argv[]) {
        // 现在需要 4 个参数：程序名、线程数、目录路径,IOSIZE,output_filepath
    if (argc != 5) {
        std::cerr << "用法: " << argv[0] << " <thread_num> <directory_path> <IOSIZE> <output_filepath>" << std::endl;
        std::cerr << "示例: " << argv[0] << " 4 /path/to/your/directory  1KB /path/to/output_file" << std::endl;
        cerr << "IOSIZE支持格式: 4K, 64K, 1M, 4M等 (K=KB, M=MB)" << endl;
        return 0;
    }

    // 从命令行参数获取线程数
    int num_threads = std::stoi(argv[1]);
    
    // 从命令行参数获取目录路径
    const std::string directory_path = argv[2];
    
    // 解析IOSIZE参数
    const char *io_size_str = argv[3];
    size_t io_size = parseIOSize(io_size_str);
    cout << "设置IOSIZE: " << io_size_str << " = " << io_size << " 字节" << endl;

    const std::string output_filepath = argv[4];


    // 这里可以使用 thread_num 和 directory_path 进行后续处理
    std::cout << "线程数: " << num_threads << std::endl;
    std::cout << "目录路径: " << directory_path << std::endl;
    std:cout << "IOSIZE: " << io_size << " 字节" << std::endl;
    std::cout << "输出文件路径: " << output_filepath << std::endl;

    // const std::string directory_path = "/mnt/beegfs/image_32KB_10428"; // 替换为你的目录路径
    // const std::string directory_path = "/mnt/beegfs/test_dir_4KB"; // 替换为你的目录路径
    // const std::string directory_path = "/mnt/beegfs/test_dir_4KB_10428"; // 替换为你的目录路径
    // const std::string directory_path = "/mnt/beegfs/test_dir/train/n02138441"; // 替换为你的目录路径
    // const std::string directory_path = "/mnt/beegfs/test_dir_1KB_10428"; // 替换为你的目录路径

    // const std::string directory_path = "/mnt/beegfs/test_dir_1MB_1000"; // 替换为你的目录路径

    // const std::string directory_path = "/mnt/beegfs/test_dir_1MB_10428"; // 替换为你的目录路径
    // const std::string directory_path = "/mnt/beegfs/test_dir_16KB_10428"; // 替换为你的目录路径



    // const std::string directory_path = "/mnt/beegfs/test_dir_32KB_500"; // 替换为你的目录路径


    // int num_threads = stoi(argv[1]); // 线程数量

    // 获取目录中的文件列表
    std::vector<std::string> files = get_files_in_directory(directory_path);
    if (files.empty()) {
        std::cerr << "目录为空或无法获取文件列表: " << directory_path << std::endl;
        return 1;
    }

    // 预先打开所有文件
    std::map<std::string, int> file_descriptors = open_all_files(directory_path, files);
    if (file_descriptors.empty()) {
        std::cerr << "没有成功打开任何文件" << std::endl;
        return 1;
    }

    // 获取每个文件的大小信息
    std::map<std::string, off_t> file_sizes = get_file_sizes(file_descriptors);



    // 创建线程
    std::vector<std::thread> threads;
    int files_per_thread = files.size() / num_threads;
    int remaining_files = files.size() % num_threads;

    int start = 0;
        // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        int end = start + files_per_thread;
        if (remaining_files > 0) {
            end += 1;
            remaining_files -= 1;
        }

        // 创建线程并传递文件范围
        threads.emplace_back(process_files, directory_path, std::cref(file_descriptors), 
                            std::cref(file_sizes), i + 1, start, end,io_size);
        start = end;
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 记录结束时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    auto total_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    auto total_time_s=total_time_ms/1000.0;
    // auto total_time_ms =total_time_us/1000.0;
    // 关闭所有文件
    close_all_files(file_descriptors);

    // 将结果写入 CSV 文件 
    
    std::ofstream csv_file(output_filepath, std::ios::app);
    
    if (csv_file.is_open()) {
        if (csv_file.tellp() == 0) {
           
            
            csv_file << "Method,Directory, ThreadNum, IOSIZE, Total Time(s),IOPS,Throughput(GBytes/s),Filenum,Avg_latancy(ms)," << std::endl;
            
        }
         
            int filenum=files.size();
            double IOPS=filenum/(total_time_s);
            double throughput_gbs=(filenum * io_size)/(total_time_s * 1024.0 * 1024.0 * 1024.0);
            double avg_latancy_ms=1.0*total_time_ms/filenum;
            csv_file << "Baseline"<<","<<directory_path << "," << num_threads << ","<<io_size_str<< "," << total_time_s << "," <<IOPS<< "," <<throughput_gbs<< "," << filenum<< "," << avg_latancy_ms << std::endl;
        
        csv_file.close();
    } else {
        std::cerr << "无法打开 CSV 文件进行写入" << std::endl;
    }
    
    std::cout << "所有线程完成! 总读取时间: " << total_time_ms << " 毫秒" << std::endl;
    return 0;
}