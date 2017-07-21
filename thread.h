//
// thread.h
//
// Define the interface for threads, an abstract class Thread.
// 
// author: Zong Wenbo
// date:   15 Mar 2017
//

#ifndef __THREAD_H_
#define __THREAD_H_

#include <memory>

class Runnable;

class Thread {
	public:
		virtual void Run(std::shared_ptr<Runnable>) = 0;
		virtual ~Thread() {} 
};


class ThreadFactory {
	public:
		virtual std::unique_ptr<Thread> NewThread() = 0;
};

#endif // __THREAD_H_
