//
// Implement a priority queue.
// 
// author: Zong Wenbo
// date:   12 April 2017
//


#ifndef __PQUEUE_H_
#define __PQUEUE_H_

#include <queue>
#include <mutex>

// A thin wrapper of std::priority_queue.
template<class T>
class PriQueue {
	public:
		PriQueue() = default;
		explicit PriQueue(uint32_t sz) {}

		void push(const T& t) {
			items_.push(t);
			return;
		}

		T pop() {
			auto t = items_.top();
			items_.pop();
			return t;
		}
	private:
		std::priority_queue<T> items_; 

};

// counter_ has to be global as Priority is a template, but we want
// all instantiated types of Priority to be numbered uniquely.
static int64_t counter = 1;
static std::mutex mtx;

// Task to work with PriQueue
class Priority {
	public:
		Priority() : priority_(0), id_(0) {}
		Priority(int priority) : priority_(priority) {
			std::lock_guard<std::mutex> lck(mtx);
			id_ = counter;
			++counter;
		}

		int GetPriority() {
			return priority_;
		}

	private:
		int priority_;
		int64_t id_;
		friend std::less<Priority>;

};

// specialize less<> for Priority
namespace std {
template<>
class less<Priority> {
	public:
		bool operator() (const Priority& x, const Priority& y) const {
			if (x.priority_ < y.priority_) {
				return true;
			}
			if (x.priority_ > y.priority_) {
				return false;
			}
			if (x.id_ < y.id_) {
				return true;
			}
			return false;
		}
};
}


#endif // __PQUEUE_H_
