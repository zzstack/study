#pragma once

#include <thread>

class ThreadNotQuitAtDll
{
	std::thread* _thread = NULL;
public:
	ThreadNotQuitAtDll();
	~ThreadNotQuitAtDll();
public:
	void Init();
private:
	void workerLoop();
};


class StackThreadNotQuitAtDll
{
	std::thread _thread;
public:
	StackThreadNotQuitAtDll();
	~StackThreadNotQuitAtDll();
public:
	void Init();
private:
	void workerLoop();
};

extern "C" {
	__declspec(dllexport) void Init();
}