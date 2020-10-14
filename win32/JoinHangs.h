#pragma once

#include <iostream>
#include <string>
#include <thread>

void SleepFor(unsigned int ms) {
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

template<typename T>
class ThreadTest {
public:
	ThreadTest() :thread_([] { SleepFor(10); }) {

	}
	ThreadTest() {
		std::cout << "About to join\t" << id() << std::endl;
		thread_.join();
		std::cout << "Joined\t\t" << id() << std::endl;
	}
private:
	std::string id()const { return typeid(decltype(thread_)).name(); }
	T thread_;
};

int main() {
	static ThreadTest<std::thread> std_test;
	return 0;
}