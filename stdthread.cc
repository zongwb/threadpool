// 
// Implementation of StdThread.
//

#include "stdthread.h"

#include <memory>
#include <future>
#include <thread>
#include <iostream>

#include "runnable.h"

using namespace std;

StdThread::StdThread(DtorAction a) : action_(a) {
}

StdThread::~StdThread() {
#ifdef VERBOSE
	cout << "StdThread DTOR " << endl;
#endif
	if (thread_->joinable()) {
		if (action_ == DtorAction::join) {
			thread_->join();
		} else {
			thread_->detach();
		}
	}
}

void StdThread::Run(std::shared_ptr<Runnable> func) {
	// Must not pass raw pointer to Runnable::run, as it will go out of scope
//	fut_ = async(launch::async, [func=func]() {
//			func->Run();
//			});
	thread_ = make_unique<std::thread>([func=func]() {
			func->Run();
			});
}

unique_ptr<Thread> StdThreadFactory::NewThread() {
	return make_unique<StdThread>(StdThread::DtorAction::join);
}
