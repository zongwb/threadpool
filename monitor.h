#ifndef __MONITOR_H_
#define __MONITOR_H_

#include <mutex>

// Implement a monitor.
class Monitor {
	public:
		explicit Monitor(std::mutex *m); 
		// Disallow copy or assignment
		Monitor(const Monitor&) = delete;
		Monitor(Monitor&&) = delete;
		Monitor& operator=(const Monitor&) = delete;
		Monitor& operator=(Monitor&&) = delete;
		~Monitor();

		std::mutex& Mutex();
		void lock();
		void unlock();
		bool wait(int64_t timeout=0);
		void notify();
		void notifyAll();

	private:
		class Impl;
		Impl *impl_;
};


#endif // __MONITOR_H_
