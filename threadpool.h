//
// threadpool.h
//
// Define the interface for ThreadPool.
// 
// author: Zong Wenbo
// date:   15 Mar 2017
//

#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <memory>
#include <vector>
#include <exception>

#include "thread.h"


const uint32_t MaxWorkers = 50;
const uint32_t MaxTasks = 200;
const uint32_t DefaultWorkers = 2;
const uint32_t DefaultQueueSize = 50;


// Threadpool is an abstract class defining an interface.
class ThreadPool {
	public:
		virtual ~ThreadPool() {}
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void StopNow() = 0;
		// Meaning of default values:
		//   timeout = -1: blocking until the task is submitted
		//   expiration = 0: task will not expire
		//   pirority = 0: lowest priority
		virtual bool Post(const std::shared_ptr<Runnable> &task, 
				int64_t timeout=-1, int64_t expiration=0, int priority=0) = 0;
		// int pendingTasks();
};


class TPException : public std::exception {
	public:
		TPException(const std::string &msg) : msg_(msg) {}
		virtual const char* what() const noexcept {
			return msg_.c_str();
		}
	private:
	std::string msg_;
};

#endif // __THREADPOOL_H_
