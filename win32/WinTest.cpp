// WinTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <Windows.h>

HMODULE loadModule(const std::wstring& fileName)
{
	HMODULE hModule = LoadLibraryW(fileName.c_str());
	if (NULL == hModule)
	{
		printf("LoadLibrary failed,%u\n", ::GetLastError());
	}
	return hModule;
}

void unloadModule(HMODULE hModule)
{
	BOOL bRet = ::FreeLibrary(hModule);
	if (!bRet)
	{
		printf("FreeLibrary failed,%d\n", ::GetLastError());
	}
	else
	{
		//printf("FreeLibrary success\n");
	}
}

/////////////////////////////////////////////////////////////////////

void testThreadNotQuitAtDll()
{
	HMODULE hModule = loadModule(L"ThreadNotQuitAtDll.dll");
	typedef void (*PFNInit)();
	PFNInit pfnInit = (PFNInit)(::GetProcAddress(hModule, "Init"));
	pfnInit();
	unloadModule(hModule);
}

void testSingletonOnDll()
{
	HMODULE hModule1 = loadModule(L"DllSingletonFirst.dll");
	HMODULE hModule2 = loadModule(L"DllSingletonSecond.dll");
	unloadModule(hModule2);
	unloadModule(hModule1);
}

#include "ThreadTest.h"

void testThread()
{
	//RunningThreadOnStackDestroyCauseException t1;
	//RunningThreadOnHeapDestroyCauseNothing t2;
	RunningCreateThreadDestroyCauseNothing t3;
}

int main2()
{
	printf("begin test...\n");
	
	testThreadNotQuitAtDll();
	
	testSingletonOnDll();

	testThread();

	printf("finish test...\n");
	return 0;
}

#include <iostream>
#include <string>
#include <thread>

void SleepFor(unsigned int ms) {
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

class ThreadTest {
public:
	ThreadTest() :thread_([] { SleepFor(1000); }) {

	}
	~ThreadTest() {
		std::cout << "About to join\t" << std::endl;
		thread_.join();
		std::cout << "Joined\t\t" << std::endl;
	}
private:
	std::thread thread_;
};

int main() {
	static ThreadTest std_test;
	//SleepFor(10);
	return 0;
}