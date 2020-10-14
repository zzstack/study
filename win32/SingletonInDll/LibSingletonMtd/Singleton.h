#pragma once

#include <stdio.h>

class LibStatisSingleton
{
	LibStatisSingleton(const LibStatisSingleton&) = delete;
	LibStatisSingleton& operator=(const LibStatisSingleton&) = delete;
private:
	unsigned int value = 0;
private:
	LibStatisSingleton() { ; }
	~LibStatisSingleton() { ; }
public:
	static LibStatisSingleton& GetInstance()
	{
		static LibStatisSingleton inst;
		return inst;
	}
public:
	void Add()
	{
		++value;
	}
	void Print()
	{
		printf("value for static singleton:%d\n", value);
	}
};


class LibLazySingleton
{
	LibLazySingleton(const LibLazySingleton&) = delete;
	LibLazySingleton& operator=(const LibLazySingleton&) = delete;
private:
	unsigned int value = 0;
private:
	LibLazySingleton() { ; }
	~LibLazySingleton() { ; }
public:
	static LibLazySingleton* GetInstance()
	{
		static LibLazySingleton* pInst = nullptr;
		if (nullptr == pInst)
		{
			pInst = new LibLazySingleton();
		}
		return pInst;
	}
public:
	void Add()
	{
		++value;
	}
	void Print()
	{
		printf("value for lazy singleton:%d\n", value);
	}
};
