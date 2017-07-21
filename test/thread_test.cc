#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>

#include "stdthread.h"

using namespace std;

struct Job : public Runnable {
	virtual void Run() override {
		cout << "Thread " << id_ << " working...\n";
		this_thread::sleep_for(chrono::seconds(5));
		cout << "Thread " << id_ << " done\n";
	}
	Job (int n) : id_(n) {}
	int id_;
};

int main() {
	int N = 2;
	StdThreadFactory factory;

	vector<unique_ptr<Thread>> pool;
	for (int i = 0; i < N; i++) {
		pool.push_back(factory.NewThread());	
	}

	for (int i = 0; i < N; i++) {
		auto job = std::make_unique<Job>(i);
		pool[i]->Run(std::move(job));
	}
/*
	auto job1 = make_shared<Job>(1);
	auto job2 = make_shared<Job>(2);
	auto thread1 = StdThread();
	auto thread2 = StdThread();
	thread1.run(job1);
	thread2.run(job2);
*/
	cout << "Starting..." << endl;
	this_thread::sleep_for(chrono::seconds(6));
	cout << "Exiting..." << endl;

	return 0;
}
