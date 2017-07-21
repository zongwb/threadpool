//
// Implement thread-safe FIIO queue.
// 
// author: Zong Wenbo
// date:   12 April 2017
//


#ifndef __CHANNEL_H_
#define __CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory.h>
#include <iostream>
#include <typeinfo>

#include "fifoqueue.h"

// Thread-safe queue. The queue 'policy' is determined by the template
// parameter Container, which is FifoQueue by default.
// The owner of a Channel object must make sure there is no outstanding
// calls to Get() or Put() upon destruction; else it may lead to SEGFAULT.
template<class T, class Container = FifoQueue<T>>
class Channel {
	public:
		explicit Channel(uint32_t sz) : closed_(false), size_(0), limit_(sz) {}
		// Disallow copy or assignment
		Channel(const Channel&) = delete;
		Channel(Channel&&) = delete;
		Channel& operator=(const Channel&) = delete;
		Channel& operator=(Channel&&) = delete;
		~Channel() {
#ifdef VERBOSE
			//std::cout << "QUEUE DTOR" << std::endl;
#endif
		}

		// Cancel all pending Get or Put.
		void Close() {
			closed_ = true;
			consume_.notify_all();
			produce_.notify_all();
		}
		
		// Get can be blocking or nonblocking, depending on the parameter  timeout.
		// If timeout == 0, it returns either one item or fails immediately without blocking.
		// If timeout < 0, it blocks indefinitely unitl an item can be returned.
		// If timeout > 0, it blocks until an item can be returned or times out after timeout milliseconds.
		// Successful calls will have an item returned; failed calls (e.g. time out) will
		// have a default-constructed T returned. 
		T Get(int64_t timeout) {
#ifdef VERBOSE
			//std::cout << "FIFO get" << std::endl;
#endif

			std::unique_lock<std::mutex> lck(mtx_);
			if (hasItem()) {
				auto item = removeItem();
				produce_.notify_one();
				return item;
			}
			// return if timed out or closed
			if (timeout == 0) {
				produce_.notify_one();
				return T();
			}
			if (closed_) {
				return T();
			}

			if (timeout < 0) {
				consume_.wait(lck, [=] {
						return closed_ || hasItem();
						});
			} else {
				if (!consume_.wait_for(lck, 
						std::chrono::milliseconds(timeout), 
						[=] {
						return closed_ || hasItem();
						})) {
					// Timed out
					produce_.notify_one();
					return T();
				}
			}

			// must not touch the queue if woken up by cancel().	
			if (closed_) {
				return T();
			}

			auto item = removeItem();
			produce_.notify_one();
			
			return item;
		}

		// Put can be blocking or nonblocking, depending on timeout.
		// If timeout == 0, it either puts the item in the queue if there's space and fails immediately.
		// If timeout < 0, it blocks indefinitely unitl the item is enqueued.
		// If timeout > 0, it blocks until the item is enqueued or times out after timeout milliseconds.
		// It returns true if the item is enqueued, false otherwise. 
		bool Put(const T &t, int64_t timeout) {
#ifdef VERBOSE
			//std::cout << "FIFO put" << std::endl;
#endif

			std::unique_lock<std::mutex> lck(mtx_);
			if (hasSpace()) {
				addItem(t);
				consume_.notify_one();
				return true;
			}
			// return if timed out or closed
			if (timeout == 0) {
				consume_.notify_one();
				return false;
			}
			if (closed_) {
				return false;
			}

			if (timeout < 0) {
				produce_.wait(lck, [=]{
						return closed_ || hasSpace();
						});
			} else {
				if (!produce_.wait_for(lck, 
						std::chrono::milliseconds(timeout), 
						[=]{
						return closed_ || hasSpace();
						})) {
					// Timed out
					consume_.notify_one();
					return false;
				}
			}
			// must not touch the queue if woken up by cancel().	
			if (closed_) {
				return false;
			}
			
			addItem(t);
			consume_.notify_one();

			return true;
		}
	private:
		std::mutex mtx_;
		std::condition_variable consume_;
		std::condition_variable produce_;
		bool closed_;

		uint32_t size_;
		const uint32_t limit_;
		Container items_; 

		inline bool hasSpace() {
			//std::cout << "check space @ " << size_ << std::endl;
			return size_ < limit_;
		}
		inline bool hasItem() {
			//std::cout << "check item @ " << size_ << std::endl;
			return size_ > 0;
		}
		inline void addItem(const T &t) {
			//std::cout << "push item @ " << size_ << std::endl;
			items_.push(t);
			++size_;
		}
		inline T removeItem() {
			//auto item = items_.front();
			//items_.pop();
			auto item = items_.pop();
			//std::cout << "remove item @ " << size_ << std::endl;
			--size_;
			return item;
		}
};

#endif // __CHANNEL_H_
