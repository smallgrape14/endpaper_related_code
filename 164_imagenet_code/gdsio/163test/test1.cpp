/*
 * Copyright 2020 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
/*
 *  This is a data-integrity test for cuFileRead/Write APIs.
 *  The test does the following:
 *  1. Creates a Test file with pattern
 *  2. Test file is loaded to device memory (cuFileRead)
 *  3. From device memory, data is written to a new file (cuFileWrite)
 *  4. Test file and new file are compared for data integrity
 *
 * ./CUFileTest005
 *
 * e9d2f73120b2f2b1d2782e8ef5a42a3259b3c2badc5edb6ee04d4bc7b7633a
 * e9d2f73120b2f2b1d2782e8ef5a42a3259b3c2badc5edb6ee04d4bc7b7633a
 * SHA SUM Match
 * API Version :
 * 440-442(us) : 1
 * 510-512(us) : 1
 *
 */
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <chrono>
#include <iostream>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <openssl/sha.h>

#include <cuda_runtime.h>

// include this header
// #include "cufile.h"
#include <cufile.h>

#include "cufile_sample_utils.h"

using namespace std;

// copy bytes
#define MAX_BUF_SIZE (31 * 1024 * 1024UL)
#define BLOCK_SIZE (4 * 1024) // 4KB
int main(int argc, char *argv[]) {
	int fd;
	ssize_t ret = -1;
	void *devPtr = NULL, *hostPtr = NULL;
	const size_t size = MAX_BUF_SIZE;
	CUfileError_t status;
        CUfileDescr_t cf_descr;
        CUfileHandle_t cf_handle;
	

	const char *file_path, *output_time_file;
    file_path = argv[1];
    output_time_file = argv[2];
	// check_cudaruntimecall(cudaSetDevice(atoi(argv[3])));
    cudaSetDevice(atoi(argv[3]));

    // 创建CUDA事件用于计时
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // 打开文件并获得文件句柄
	// Load Test file to GPU memory
	fd = open(file_path, O_RDONLY | O_DIRECT);
	if (fd < 0) {
		std::cerr << "read file open error : " << file_path << " "
			<< std::strerror(errno) << std::endl;
		return -1;
	}

        memset((void *)&cf_descr, 0, sizeof(CUfileDescr_t));
        cf_descr.handle.fd = fd;
        cf_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
        status = cuFileHandleRegister(&cf_handle, &cf_descr);
        if (status.err != CU_FILE_SUCCESS) {
                std::cerr << "file register error: "
			// << cuFileGetErrorString(status) << std::endl;
            // << cuGetErrorString(status) 
            << std::endl;
            
                close(fd);
                return -1;
        }
    //分配GPU内存用于存储读取的数据
	// check_cudaruntimecall(cudaMalloc(&devPtr, size));
    cudaMalloc(&devPtr, size);
	// special case for holes
	// check_cudaruntimecall(cudaMemset(devPtr, 0, size));
    cudaMemset(devPtr, 0, size);
	// check_cudaruntimecall(cudaStreamSynchronize(0));
    cudaStreamSynchronize(0);
	std::cout << "reading file to device memory :" << file_path
				<< std::endl;


    //
    // FILE* fp = fopen(output_time_file, "w");
    FILE* fp = fopen(output_time_file, "a");
    if (!fp) {
        fprintf(stderr, "Failed to open output file.\n");
        // cudaFree(d_buffer);
        // cuFileHandleDeregister(file_handle);
        return 1;
    }

    //// 循环读取文件并测量时间
    long int index=0,ops=0;
    ssize_t bytes_read=-1;
    float elapsedTime_total=0;
    for (size_t offset = 0;index<=ops ; offset += BLOCK_SIZE) {
        // 记录读取开始时间
        cudaEventRecord(start, 0);
        // 执行同步读取操作
        // bytes_read=cuFileRead(file_handle, devPtr, BLOCK_SIZE, offset, CU_FILE_HANDLE_FLAGS_NONE);
        // bytes_read=cuFileRead(file_handle, devPtr, BLOCK_SIZE, offset,0);

        // bytes_read=cuFileRead(file_handle,devPtr,BLOCK_SIZE,offset,0);
        bytes_read = cuFileRead(cf_handle, devPtr, BLOCK_SIZE, offset, 0);
        // if (bytes_read < 0) {
        //     fprintf(stderr, "Error reading from file.\n");
        //     break;
        // }
        // 记录读取结束时间
        cudaEventRecord(stop, 0);
        cudaEventSynchronize(stop);
        if (bytes_read < 0) {
            if (IS_CUFILE_ERR(bytes_read))
                std::cerr << "read failed : "
                    << cuFileGetErrorString(ret) << std::endl;
            else
                std::cerr << "read failed : "
                    << cuFileGetErrorString(errno) << std::endl;
            break;
	    }

        // // 记录读取结束时间
        // cudaEventRecord(stop, 0);
        // cudaEventSynchronize(stop);

        // 计算经过的时间
        float elapsedTime;
        cudaEventElapsedTime(&elapsedTime, start, stop);// ms 级别的时间

        // 将时间记录到文件
        fprintf(fp, "%f\n", elapsedTime);

        elapsedTime_total+=elapsedTime;
        index++;
    }
    float  elapsedTime_avg=elapsedTime_total/ops;
    // fprintf(fp, "AVg: %f\n", elapsedTime_avg);

    /*
	ret = cuFileRead(cf_handle, devPtr, size, 0, 0);
	if (ret < 0) {
		if (IS_CUFILE_ERR(ret))
			std::cerr << "read failed : "
				<< cuFileGetErrorString(ret) << std::endl;
		else
			std::cerr << "read failed : "
				<< cuFileGetErrorString(errno) << std::endl;
		cuFileHandleDeregister(cf_handle);
		close(fd);
		check_cudaruntimecall(cudaFree(devPtr));
		return -1;
	}*/



    cudaEventDestroy(start);
    cudaEventDestroy(stop);
	// check_cudaruntimecall(cudaFree(devPtr));
    cudaFree(devPtr);
	cuFileHandleDeregister(cf_handle);
	close(fd);
	// close(fp);

	return 0;
}