#include "monitor.h"

#include <iostream>
#include <condition_variable>

using namespace std;

class Monitor::Impl {
	public:
		Impl () : mtx_(nullptr), ownedMtx_(new std::mutex()) {
			init(ownedMtx_);
		}
		explicit Impl (std::mutex *m) : mtx_(nullptr), ownedMtx_(nullptr) {
			init(m);
		}
		~Impl() {
			cleanup();
		}
		
		void lock() {
			mtx_->lock();
		}

		void unlock() {
			mtx_->unlock();
		}

		bool wait(int64_t timeout) {
			if (timeout == 0) {
				cv_.wait(*lck_);
			} else {
				if (cv_.wait_for(*lck_, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
					return false;
				}
			}
			return true;
		}

		void notify() {
			cv_.notify_one();
		}

		void notifyAll() {
			cv_.notify_all();
		}

	private:
		std::mutex *mtx_;
		std::mutex *ownedMtx_;
		std::unique_lock<std::mutex> *lck_;
		std::condition_variable cv_;
		
		void init(std::mutex *m) {
			mtx_ = m;
			lck_ = new std::unique_lock<std::mutex>(*mtx_, std::defer_lock);
		}

		void cleanup() {
			delete lck_; // must destroy lck_ before ownedMtx_
			delete ownedMtx_;
		}
};

Monitor::Monitor(std::mutex *m) : impl_(new Impl(m)) {}

Monitor::~Monitor() {
	delete impl_;
}

void Monitor::lock() {
	impl_->lock();
}

void Monitor::unlock() {
	impl_->unlock();
}

bool Monitor::wait(int64_t timeout) {
	return impl_->wait(timeout);
}

void Monitor::notify() {
	impl_->notify();
}

void Monitor::notifyAll() {
	impl_->notifyAll();
}

