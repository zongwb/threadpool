#ifndef __SEMAPHORE_H_
#define __SEMAPHORE_H_

#include <mutex>
#include <condition_variable>
#include <iostream>

template<class Mutex, class CondVar>
class basic_semaphore {
	public:
		explicit basic_semaphore(int c = 0) : count_(c) {	
		}
		basic_semaphore(const basic_semaphore&) = delete;
		basic_semaphore(basic_semaphore&&) = delete;
		basic_semaphore& operator=(const basic_semaphore&) = delete;
		basic_semaphore& operator=(basic_semaphore&&) = delete;
		void Wait() {
			std::unique_lock<std::mutex> lck(mtx_);
			while (count_ == 0) { // handle spurious wakeup
				//std::cout << "Semaphore wait" << std::endl;
				cv_.wait(lck);
			}
			//cv_.wait(lck, [&]{ return count_ > 0;});
			--count_;
		}
		void TryWait() {
			std::unique_lock<std::mutex> lck(mtx_);
			if (count_ > 0) {
				--count_;
				return true;
			}
			return false;
		}
		void Notify() {
			std::unique_lock<std::mutex> lck(mtx_);
			++count_;
			//std::cout << "Semaphore Notify" << std::endl;
			cv_.notify_one();
		}

	private:
		Mutex mtx_;
		CondVar cv_;
		int count_;
};

using Semaphore = basic_semaphore<std::mutex, std::condition_variable>;

#endif // __SEMAPHORE_H_
