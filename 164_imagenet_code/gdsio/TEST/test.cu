// To compile this sample code:
//
// nvcc test.cu -o test -lcufile
//nvcc -std=c++11 -O2 test.cu -o test -lcufile -lcudart
//nvcc -std=c++11 -O2 -ccbin /usr/bin/gcc-11 -allow-unsupported-compiler test.cu -o test -lcufile -lcudart
// Set the environment variable TESTFILE
// to specify the name of the file on a GDS enabled filesystem
//
// Ex:   TESTFILE=/mnt/gds/gds_test ./gds_helloworld
//
//
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cuda_runtime.h>
#include "cufile.h"

//#include "cufile_sample_utils.h"
using namespace std;

int main(void) {
        int fd;
        ssize_t ret;
        void *devPtr_base;
        off_t file_offset = 0x2000;
        off_t devPtr_offset = 0x1000;
        ssize_t IO_size = 1UL << 24;
        size_t buff_size = IO_size + 0x1000;
        CUfileError_t status;
        // CUResult cuda_result;
        int cuda_result;
        CUfileDescr_t cf_descr;
        CUfileHandle_t cf_handle;
        char *testfn;

        testfn=getenv("TESTFILE");
        if (testfn==NULL) {
            std::cerr << "No testfile defined via TESTFILE.  Exiting." << std::endl;
            return -1;
        }

        cout << std::endl;
        cout << "Opening File " << testfn << std::endl;

        fd = open(testfn, O_CREAT|O_WRONLY|O_DIRECT, 0644);
        if(fd < 0) {
                std::cerr << "file open " << testfn << "errno " << errno << std::endl;
                return -1;
        }

        // the above fd could also have been opened without O_DIRECT starting CUDA toolkit 12.2
        // (gds 1.7.x version) as follows
        // fd = open(testfn, O_CREAT|O_WRONLY, 0644);

        cout << "Opening cuFileDriver." << std::endl;
        status = cuFileDriverOpen();
        if (status.err != CU_FILE_SUCCESS) {
                std::cerr << " cuFile driver failed to open " << std::endl;
                close(fd);
                return -1;
        }

        cout << "Registering cuFile handle to " << testfn << "." << std::endl;

        memset((void *)&cf_descr, 0, sizeof(CUfileDescr_t));
        cf_descr.handle.fd = fd;
        cf_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
        status = cuFileHandleRegister(&cf_handle, &cf_descr);
        if (status.err != CU_FILE_SUCCESS) {
                std::cerr << "cuFileHandleRegister fd " << fd << " status " << status.err << std::endl;
                close(fd);
                return -1;
        }

        cout << "Allocating CUDA buffer of " << buff_size << " bytes." << std::endl;

        cuda_result = cudaMalloc(&devPtr_base, buff_size);
        if (cuda_result != CUDA_SUCCESS) {
                std::cerr << "buffer allocation failed " << cuda_result << std::endl;
                cuFileHandleDeregister(cf_handle);
                close(fd);
                return -1;
        }

        cout << "Registering Buffer of " << buff_size << " bytes." << std::endl;
        status = cuFileBufRegister(devPtr_base, buff_size, 0);
        if (status.err != CU_FILE_SUCCESS) {
                std::cerr << "buffer registration failed " << status.err << std::endl;
                cuFileHandleDeregister(cf_handle);
                close(fd);
                cudaFree(devPtr_base);
                return -1;
        }

        // fill a pattern
        cout << "Filling memory." << std::endl;

        cudaMemset((void *) devPtr_base, 0xab, buff_size);
        cuStreamSynchronize(0);

        // perform write operation directly from GPU mem to file
        cout << "Writing buffer to file." << std::endl;
        ret = cuFileWrite(cf_handle, devPtr_base, IO_size, file_offset, devPtr_offset);

        if (ret < 0 || ret != IO_size) {
                std::cerr << "cuFileWrite failed " << ret << std::endl;
        }

        // release the GPU memory pinning
        cout << "Releasing cuFile buffer." << std::endl;
        status = cuFileBufDeregister(devPtr_base);
        if (status.err != CU_FILE_SUCCESS) {
                std::cerr << "buffer deregister failed" << std::endl;
                cudaFree(devPtr_base);
                cuFileHandleDeregister(cf_handle);
                close(fd);
                return -1;
        }

        cout << "Freeing CUDA buffer." << std::endl;
        cudaFree(devPtr_base);
        // deregister the handle from cuFile
        cout << "Releasing file handle. " << std::endl;
        (void) cuFileHandleDeregister(cf_handle);
        close(fd);

        // release all cuFile resources
        cout << "Closing File Driver." << std::endl;
        (void) cuFileDriverClose();

        cout << std::endl;

        return 0;
}