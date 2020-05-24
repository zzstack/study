#include "OPenCLUtil.h"

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include "common/PathUtil.h"

using namespace std;

OpenClAdapter::OpenClAdapter() {

}

OpenClAdapter::~OpenClAdapter() {

}

void OpenClAdapter::Init() {
    // Step 1: Getting platforms and choose an available one(first)
    std::string error = "";
    if(!initPlatform(_platform,error)){
        return;
    }

    // Step 2:Query the platform and choose the first GPU device if has one.
    _devices = initDevice(_platform,error);
    if(nullptr == _devices) {
        return;
    }

    // Step 3: Create context.
    _context = clCreateContext(NULL,1, _devices,NULL,NULL,NULL);

    // Step 4: Creating command queue associate with the context.
    _commandQueue = clCreateCommandQueue(_context, _devices[0], 0, NULL);
}

void OpenClAdapter::Uninit() {
    cl_int status = clReleaseCommandQueue(_commandQueue);//Release  Command queue.
    status = clReleaseContext(_context);//Release context.

    if (nullptr != _devices) {
        free(_devices);
        _devices = nullptr;
    }
}

void OpenClAdapter::RunTest(const char* funcName,const char* fileName) {
     // Step 5: Create program object
    string sourceStr;
    std::string file = "";
    BYTEVIEW_COMMON::PathUtil::GetAbsDirPath(fileName,file);
    file = "/Users/zengjj/Product/pc-sdk-fixbug/byteview-crypto/src/SimpleEncryptKernel.cl";
    cl_int status = convertToString(file.c_str(), sourceStr);

    StartTask(funcName,sourceStr.c_str());

    // Step 7: Initial input,output for the host and create memory objects for the kernel
    static const int count = 10;
    unsigned int* input = new unsigned int[count];
    for(int i=0;i<count;i++){
        input[i]=i;
    }
    unsigned int* output = new unsigned int[count];

    RunTask((void *) input,count*sizeof(unsigned int),output,count*sizeof(unsigned int),count);
    StopTask();

    std::cout << "output:";
    for(int i = 0;i < count;++i) {
        std::cout << output[i] << ",";
    }
    std::cout << std::endl;
    if (output != NULL)
    {
        free(output);
        output = NULL;
    }

}

void OpenClAdapter::StartTask(const char* funcName,const char* source) {
   size_t sourceSize[] = { strlen(source) };
    _program = clCreateProgramWithSource(_context, 1, &source, sourceSize, NULL);

    // Step 6: Build program.
    cl_int status = clBuildProgram(_program, 1,_devices,NULL,NULL,NULL);

    if (CL_SUCCESS != status) {
        std::cout << "clBuildProgram failed!" << std::endl;
        return;
    }

    // Step 8: Create kernel object
    _kernel = clCreateKernel(_program,funcName, NULL);
}

void OpenClAdapter::RunTask(void* inBuffer,size_t inSize,void* outBuffer,size_t outSize,size_t workSize) {

    cl_mem inputBuffer = clCreateBuffer(_context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, inSize,inBuffer, NULL);
    cl_mem outputBuffer = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, outSize, NULL, NULL);

    // Step 9: Sets Kernel arguments
    cl_int status = clSetKernelArg(_kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
    status = clSetKernelArg(_kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);

    // Step 10: Running the kernel
    size_t global_work_size[1] = { workSize };
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(_commandQueue, _kernel, 1, NULL, global_work_size, NULL, 0, NULL, &enentPoint);
    clWaitForEvents(1,&enentPoint);
    clReleaseEvent(enentPoint);

    // Step 11: Read the cout put back to host memory
    status = clEnqueueReadBuffer(_commandQueue, outputBuffer, CL_TRUE, 0, outSize, outBuffer, 0, NULL, NULL);

    status = clReleaseMemObject(inputBuffer);
    status = clReleaseMemObject(outputBuffer);
}

void OpenClAdapter::RunTask(void* buffer,size_t size,size_t workSize) {
    cl_mem inputBuffer = clCreateBuffer(_context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR|CL_MEM_WRITE_ONLY, size,buffer, NULL);

    // Step 9: Sets Kernel arguments
    cl_int status = clSetKernelArg(_kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);

    // Step 10: Running the kernel
    size_t global_work_size[1] = { workSize };
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(_commandQueue, _kernel, 1, NULL, global_work_size, NULL, 0, NULL, &enentPoint);
    clWaitForEvents(1,&enentPoint); ///wait
    clReleaseEvent(enentPoint);

    // Step 11: Read the cout put back to host memory
    status = clEnqueueReadBuffer(_commandQueue, inputBuffer, CL_TRUE, 0, size, buffer, 0, NULL, NULL);

    status = clReleaseMemObject(inputBuffer);
}

void OpenClAdapter::RunTask(void** buffer,size_t* size,size_t count,size_t workSize) {
    cl_mem* inputBuffer = new cl_mem[count];
    for(size_t i = 0;i < count;++i) {
        inputBuffer[i] = clCreateBuffer(_context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR|CL_MEM_WRITE_ONLY, size[i],buffer[i], NULL);
    }

    // Step 9: Sets Kernel arguments
    cl_int status;
    for(size_t i = 0;i < count;++i) {
        status = clSetKernelArg(_kernel, i, sizeof(cl_mem), (void *)&inputBuffer[i]);
    }

    // Step 10: Running the kernel
    size_t global_work_size[1] = { workSize };
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(_commandQueue, _kernel, 1, NULL, global_work_size, NULL, 0, NULL, &enentPoint);
    clWaitForEvents(1,&enentPoint);
    clReleaseEvent(enentPoint);

    // Step 11: Read the cout put back to host memory
    status = clEnqueueReadBuffer(_commandQueue, inputBuffer[0], CL_TRUE, 0, size[0], buffer[0], 0, NULL, NULL);

    for(size_t i = 0;i < count;++i) {
        status = clReleaseMemObject(inputBuffer[i]);
    }
    delete[] inputBuffer;
}

void OpenClAdapter::StopTask() {
    cl_int status = clReleaseKernel(_kernel);
    status = clReleaseProgram(_program);
}

int OpenClAdapter::convertToString(const char *filename, std::string& s)
{
    size_t size;
    char*  str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size+1];
        if(!str)
        {
            f.close();
            return 0;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    cout<<"Error: failed to open file\n:"<<filename<<endl;
    return -1;
}

bool OpenClAdapter::initPlatform(cl_platform_id &platform,std::string& error) {
    platform = NULL;

    cl_uint numPlatforms;
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (CL_SUCCESS != status) {
        error = "clGetPlatformIDs failed!";
        return false;
    }

    // For clarity, choose the first available platform
    if(numPlatforms <= 0) {
        return false;
    }
    cl_platform_id* platforms =
        (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    platform = platforms[0];
    free(platforms);
    return true;
}

/**Step 2:Query the platform and choose the first GPU device if has one.*/
cl_device_id* OpenClAdapter::initDevice(const cl_platform_id& platform,std::string& error) {
    cl_uint numDevices = 0;
    
    cl_int status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
    if (CL_SUCCESS != status) {
        error = "clGetDeviceIDs count failed!";
        return nullptr;
    }
    if (numDevices <= 0) {
        return nullptr;
    }
    //GPU available
    cl_device_id *devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
    if (CL_SUCCESS != status) {
        error = "clGetDeviceIDs data failed!";
        free(devices);
        return nullptr;
    }
    return devices;
}

void OpenClAdapter::uninitDevice(cl_device_id* devices) {
}