#pragma once

#include <thread>
#include <Windows.h>

class RunningThreadOnStackDestroyCauseException
{
	std::thread _thread;
public:
	RunningThreadOnStackDestroyCauseException()
	{
		printf("RunningThreadOnStackDestroyCauseException contrust...\n");
		Init();
	}
	~RunningThreadOnStackDestroyCauseException()
	{
		printf("RunningThreadOnStackDestroyCauseException destroy...\n");
	}
public:
	void Init()
	{
		_thread = std::thread(&RunningThreadOnStackDestroyCauseException::workerLoop, this);
		printf("RunningThreadOnStackDestroyCauseException threadId:%08x\n", _thread.get_id());
	}
private:
	void workerLoop()
	{
		while (1) {};
	}
};

class RunningThreadOnHeapDestroyCauseNothing
{
	std::thread* _thread = nullptr;
public:
	RunningThreadOnHeapDestroyCauseNothing()
	{
		printf("RunningThreadOnHeapDestroyCauseNothing contrust...\n");
		Init();
	}
	~RunningThreadOnHeapDestroyCauseNothing()
	{
		printf("RunningThreadOnHeapDestroyCauseNothing destroy...\n");
	}
public:
	void Init()
	{
		if (nullptr != _thread)
		{
			return;
		}
		_thread = new std::thread(&RunningThreadOnHeapDestroyCauseNothing::workerLoop, this);
		printf("RunningThreadOnHeapDestroyCauseNothing threadId:%08x\n", _thread->get_id());
	}
private:
	void workerLoop()
	{
		while (1) {};
	}
};

class RunningCreateThreadDestroyCauseNothing
{
	HANDLE	_thread = NULL;
public:
	RunningCreateThreadDestroyCauseNothing()
	{
		printf("RunningCreateThreadDestroyCauseNothing contrust...\n");
		Init();
	}
	~RunningCreateThreadDestroyCauseNothing()
	{
		printf("RunningCreateThreadDestroyCauseNothing destroy...\n");
	}
public:
	void Init()
	{
		if (NULL != _thread)
		{
			return;
		}
		DWORD threadId = 0;
		_thread = CreateThread(NULL,0,workerLoop,0,0,&threadId);
		printf("RunningCreateThreadDestroyCauseNothing threadId:%08x\n", threadId);
	}
private:
	static DWORD WINAPI workerLoop(LPVOID p)
	{
		while (1) {};
	}
};