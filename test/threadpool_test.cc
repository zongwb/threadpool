#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>

#include "stdthread.h"
#include "threadpool_impl.h"
#include "tokenbucket.h"

using namespace std;

struct Job : public Runnable {
	virtual void Run() override {
		cout << "Job " + to_string(id_) + " working...\n";// << endl;
		this_thread::sleep_for(chrono::seconds(1));
		cout << "Job " + to_string(id_) + " done\n"; // << endl;
	}
	void Print() {
		cout << "Job ID = " << id_ << endl;
	}
	Job (int n) : id_(n) {}
	int id_;
};

const uint32_t rate = 2;

int main() {
	
	auto tb = make_shared<TokenBucket>(rate);
	auto factory = make_shared<StdThreadFactory>();
	auto pool = make_unique<PriThreadPool>(factory, tb, 20, 20); // 5 workers, 2-item buffer
	pool->Start();

	int M = 10;
	for (int i = 0; i < M; ++i) {
		auto job = make_shared<Job>(i);
		pool->Post(job, 500, 5000);
	}

	cout << "Starting..." << endl;
	this_thread::sleep_for(chrono::seconds(1));
	cout << "Stopping..." << endl;
	pool->Stop();
	//pool->StopNow();
	cout << "Exiting..." << endl;

	return 0;
}
