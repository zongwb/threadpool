//
// thread.h
//
// Implement Thread using STL::thread.
// 
// author: Zong Wenbo
// date:   15 Mar 2017
//

#ifndef __STDTHREAD_H_
#define __STDTHREAD_H_

#include <future>

#include "thread.h"
#include "runnable.h"

class StdThread : public Thread {
	public:
		enum class DtorAction {join, detach};

		explicit StdThread(DtorAction a = DtorAction::join);
		virtual ~StdThread();
		StdThread(StdThread&& rhs) = default; // allow move
		StdThread& operator=(StdThread&& rhs) = default;  // allow move assignment
		StdThread(const StdThread & rhs) = delete; // no copy
		StdThread& operator=(const StdThread& rhs) = delete; // no assignment 

		virtual void Run(std::shared_ptr<Runnable>) override;
		std::thread& get();

	private:
		//std::future<void> fut_;
		DtorAction action_;
		std::unique_ptr<std::thread> thread_;
};


class StdThreadFactory : public ThreadFactory {
	public:
		virtual std::unique_ptr<Thread> NewThread() override;
};


#endif // __STDTHREAD_H_
