#ifndef _BYTEVIEWCRYPTO_OPENCLUTIL_H_
#define _BYTEVIEWCRYPTO_OPENCLUTIL_H_

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

using namespace std;

class OpenClAdapter {
    OpenClAdapter(const OpenClAdapter&) = delete;
    OpenClAdapter& operator=(const OpenClAdapter&) = delete;
private:
    cl_platform_id      _platform;
    cl_context          _context;
    cl_device_id*       _devices = nullptr;
    cl_command_queue    _commandQueue;
    cl_program          _program;
    cl_kernel           _kernel;
public:
    OpenClAdapter();
    ~OpenClAdapter();
public:
    void Init();
    void Uninit();
    
    void StartTask(const char* funcName,const char* source);
    void RunTask(void* inBuffer,size_t inSize,void* outBuffer,size_t outSize,size_t workSize);
    void RunTask(void* buffer,size_t size,size_t workSize);
    void RunTask(void** buffer,size_t* size,size_t count,size_t workSize);
    void StopTask();

    void RunTest(const char* funcName,const char* fileName);
public:
    // convert the kernel file into a string
    int convertToString(const char *filename, std::string& s);

    // Getting platforms and choose an available one.
    static bool initPlatform(cl_platform_id& platform,std::string& error);

    // Query the platform and choose the first GPU device if has one
    static cl_device_id *initDevice(const cl_platform_id& platform,std::string& error);
    static void uninitDevice(cl_device_id* devices);
};

#endif
