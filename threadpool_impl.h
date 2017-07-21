//
// Implement ThreadPool.
// 
// author: Zong Wenbo
// date:   15 Mar 2017
//
//
#ifndef __THREADPOOL_IMPL_H_
#define __THREADPOOL_IMPL_H_

#include "threadpool.h"

#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
//#include <chrono>

#include "thread.h"
#include "runnable.h"
#include "channel.h"
#include "def.h"
#include "semaphore.h"
#include "tokenbucket.h"
#include "priqueue.h"


#define MAX_THREADS (NUMCORES * 10)

const TPException kWrongCntEcp("number of threads cannot exceed NUMCORES*10");
static const int64_t kBlockingFlag = -1;

using namespace std;
using namespace std::chrono;


class Task : public Runnable {
	public:
		Task() {}
		Task(const std::shared_ptr<Runnable> &t, int64_t e, int p) : 
			task_(t),
			expiration_(e),
			priority_(p)
		{
			start_ = system_clock::now();
		}
		Task(const Task&) = default; // allow copy
		Task(Task &&) = default; // allow move
		Task& operator=(const Task&) = default; // allow copy assignment
		Task& operator=(Task&&) = default; // allow move assignment

		virtual ~Task() = default; 

		virtual void Run() override {
			if (IsExpired()) {
				return;
			}
			task_->Run();
			return; 
		}


		bool IsExpired() {
			auto now = system_clock::now();
			if (expiration_.count() > 0 && now >= start_ + expiration_) {
				return true;
			}
			return false;
		}

		bool IsEmpty() {
			if (task_ == nullptr) {
				return true;
			}
			return false;
		}

	private:
		std::shared_ptr<Runnable> task_;
		std::chrono::system_clock::time_point start_; // start time
		std::chrono::milliseconds expiration_;
		Priority priority_;
		friend std::less<Task>;
};

// specialize less<> for Task
namespace std {
template<>
class less<Task> {
	public:
		bool operator() (const Task& x, const Task& y) const {
			return std::less<Priority>()(x.priority_, y.priority_);
		}
};
}

// Worker is the consumer of the task queue. 
template<class Container>
class Worker : public Runnable {
	public:
		Worker(Container &tasks, std::shared_ptr<RateLimiter> rl=nullptr) : 
			tasks_(tasks),
			ratelimiter_(rl),
			quit_(false),
			sem_(0),
			status_(Status::STOPPED)
		{
			std::cout << "New worker " << std::endl;
		}
		~Worker() {
			std::cout << "Worker DTOR" << std::endl;
			if (!quit_) {
				stop();
				wait();
			}
		}

		// stop stops the worker when all pending tasks in the queue are processed.
		void stop() {
			quit_ = true;
			status_ = Status::STOPPING;
		}

		// stopNow stops the worker immediately and discard any pending tasks in the queue.
		void stopNow() {
			quit_ = true;
			status_ = Status::STOPPED;
		}

		void wait() {
			sem_.Wait();
			status_ = Status::STOPPED;
		}

		virtual void Run() override {
			status_ = Status::RUNNING;
			while(status_ == Status::RUNNING || status_ == Status::STOPPING) {
				auto task = tasks_.Get(kBlockingFlag); // blocking get
				if (task.IsEmpty()) {
				//if (false) {
					std::cout << "Worker no task available\n";// << std::endl;
					if (status_ == Status::STOPPING) { // queue exhausted, break out of the loop
						break;
					}
					continue;
				}
				std::cout << "Worker got a task \n";// << std::endl;
				if (task.IsExpired()) {
					std::cout << "Worker task expired \n";
					continue;
				}
				if (ratelimiter_ != nullptr && !ratelimiter_->GetToken(kBlockingFlag)) {
					std::cout << "Worker no token \n";
					continue;
				}
				if (task.IsExpired()) { // check expiry again as GetToken may take time
					std::cout << "Worker task expired 2\n";
					continue;
				}
				task.Run();
			}
			sem_.Notify();
			return; 
		}
		
		virtual void Print() {
		}

	private:
		Container &tasks_;
		std::shared_ptr<RateLimiter> ratelimiter_;
		bool quit_;
		Semaphore sem_; // for sync upen destruction
		
		enum class Status { STOPPED, RUNNING, STOPPING};
		Status status_;
};


class RateLimiter;

// Template class Threadpool depends on class Container which should
// provide the following methods:
// - Container(uint32_t sz)
// - Get()
// - Put()
// - Close()
// Importantly, these methods must be thread-safe.
template<class T, class Container>
class ThreadPoolImpl : public ThreadPool {
	using WorkerType = Worker<Container>;
	public: 
		//friend class Task;
		ThreadPoolImpl(std::shared_ptr<ThreadFactory> factory, uint32_t threads, uint32_t maxTasks) : 
			factory_(factory),
			numThreads_(threads),
			tasks_(maxTasks),
			ratelimiter_(nullptr),
			status_(Status::STOPPED){
			if (threads > MAX_THREADS) {
				throw kWrongCntEcp; 
			}
		}
		ThreadPoolImpl(std::shared_ptr<ThreadFactory> factory, std::shared_ptr<RateLimiter> rl, uint32_t threads, uint32_t maxTasks) : 
			factory_(factory),
			numThreads_(threads),
			tasks_(maxTasks),
			ratelimiter_(rl),
			status_(Status::STOPPED){
			if (threads > MAX_THREADS) {
				throw kWrongCntEcp; 
			}
		}
		~ThreadPoolImpl() {
			Stop();
		}

		virtual void Start() override {
			// TODO: guard status
			if (status_ != Status::STOPPED) {
				return;
			}
			// create threads
			threads_.reserve(numThreads_);
			for (uint32_t i = 0; i < numThreads_; ++i) {
				threads_.push_back(std::move(factory_->NewThread()));
			}

			// create workers
			for (uint32_t i = 0; i < numThreads_; ++i) {
				workers_.push_back(std::make_shared<WorkerType>(tasks_, ratelimiter_));	
			}

			// start threads
			for (uint32_t i = 0; i < numThreads_; ++i) {
				threads_[i]->Run(workers_[i]);
			}
			
			if (ratelimiter_ != nullptr) {
				ratelimiter_->Start();
			}

			status_ = Status::RUNNING;
		}

		// stop accepting new tasks, process pending tasks and shutdown the workers.
		virtual void Stop() override {
			if (status_ == Status::STOPPED || status_ == Status::STOPPING) {
				return;
			}
			status_ = Status::STOPPING;

			tasks_.Close(); // close the queue so that blocking Get can return.
			for (uint32_t i = 0; i < numThreads_; ++i) {
				workers_[i]->stop();
			}
			for (uint32_t i = 0; i < numThreads_; ++i) {
				workers_[i]->wait();
			}
			// only stop the rate limiter after all pending tasks are processed.
			if (ratelimiter_ != nullptr) {
				ratelimiter_->Stop(); // stop the rate limiter to avoid blocking
			}
			status_ = Status::STOPPED;
		}

		// stop all workers and discard any pending tasks.
		virtual void StopNow() override {
			if (status_ == Status::STOPPED || status_ == Status::STOPPING) {
				return;
			}
			status_ = Status::STOPPING;

			tasks_.Close(); // close the queue so that blocking Get can return.
			if (ratelimiter_ != nullptr) {
				ratelimiter_->Stop(); // stop the rate limiter to avoid blocking
			}
			for (uint32_t i = 0; i < numThreads_; ++i) {
				workers_[i]->stop();
			}
			for (uint32_t i = 0; i < numThreads_; ++i) {
				workers_[i]->wait();
			}
			status_ = Status::STOPPED;
		}

		// post is the producer of the task queue.
		virtual bool Post(const std::shared_ptr<Runnable> &task, int64_t timeout = -1, int64_t expiration = 0, int priority = 0) override {
			if (status_ != Status::RUNNING) {
				return false;
			}
			auto t  = T(task, expiration, priority); 
			return tasks_.Put(t, timeout);
		}
	private:
		std::shared_ptr<ThreadFactory> factory_;
		uint32_t numThreads_; 
		std::vector<std::shared_ptr<WorkerType>> workers_;
		std::vector<std::shared_ptr<Thread>> threads_;
		Container tasks_;
		std::shared_ptr<RateLimiter> ratelimiter_;

		enum class Status { STOPPED, RUNNING, STOPPING};
		Status status_;
};


using FifoThreadPool = ThreadPoolImpl<Task, Channel<Task>>;
using PriThreadPool = ThreadPoolImpl<Task, Channel<Task, PriQueue<Task>>>;
//FifoThreadPool dummy(nullptr, 1, 1);
//PriThreadPool dummy2(nullptr, 1, 1);

#endif // __THREADPOOL_IMPL_H_
