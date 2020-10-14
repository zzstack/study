// ThreadNotQuitAtDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ThreadNotQuitAtDll.h"

ThreadNotQuitAtDll* g_inst = new ThreadNotQuitAtDll();
StackThreadNotQuitAtDll g_stask;

void Init()
{
	g_inst->Init();
	g_stask.Init();
}

ThreadNotQuitAtDll::ThreadNotQuitAtDll()
{
	printf("ThreadNotQuitAtDll contrust...\n");
	//Init();
}

ThreadNotQuitAtDll::~ThreadNotQuitAtDll()
{
	printf("ThreadNotQuitAtDll destroy...\n");
}

void ThreadNotQuitAtDll::Init()
{
	_thread = new std::thread(&ThreadNotQuitAtDll::workerLoop, this);
	printf("ThreadNotQuitAtDll threadId:%08x\n", _thread->get_id());
}

void ThreadNotQuitAtDll::workerLoop()
{
	while (1) {};
}

/////////////////////////////////////////////////////

StackThreadNotQuitAtDll::StackThreadNotQuitAtDll()
{
	printf("ThreadNotQuitAtDll contrust...\n");
	//Init();
}

StackThreadNotQuitAtDll::~StackThreadNotQuitAtDll()
{
	printf("ThreadNotQuitAtDll destroy...\n");
}

void StackThreadNotQuitAtDll::Init()
{
	_thread = std::thread(&StackThreadNotQuitAtDll::workerLoop, this);
	printf("ThreadNotQuitAtDll threadId:%08x\n", _thread.get_id());
}

void StackThreadNotQuitAtDll::workerLoop()
{
	while (1) {};
}