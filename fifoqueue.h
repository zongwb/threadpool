//
// Implement a simple FIIO queue.
// 
// author: Zong Wenbo
// date:   12 April 2017
//


#ifndef __FIFOQUEUE_H_
#define __FIFOQUEUE_H_

#include <queue>

// FIFO queue, not thread-safe.
template<class T>
class FifoQueue {
	public:

		void push(const T &t) {
			items_.push(t);
		}

		T pop() {
			auto item = items_.front();
			items_.pop();
			return item;
		}

		size_t size() {
			return items_.size();
		}

	private:
		std::queue<T> items_; 

};

#endif // __FIFOQUEUE_H_
