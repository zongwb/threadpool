#include "tokenbucket.h"

#include <iostream>       // std::cout
#include <thread>         // std::this_thread::sleep_until
#include <chrono>         // std::chrono::system_clock
#include <mutex>
#include <memory>
#include <ctime>

#include "runnable.h"
#include "thread.h"
#include "stdthread.h"
#include "channel.h"
#include "priqueue.h"

struct token {
	token() : tick_(0) {}
	explicit token(uint64_t t, int p) : tick_(t), priority(p) {}
	uint64_t tick_;
	inline bool isEmpty() {
		return tick_ == 0;
	}
	void Print() {}
	int GetPriority() {
		return priority.GetPriority();
	}
	Priority priority;
};


// specialize less<> for Task
namespace std {
template<>
class less<token> {
	public:
		bool operator() (const token& x, const token& y) const {
			return std::less<Priority>()(x.priority, y.priority);
		}
};
}

class TokenBucket::Impl : public Runnable, std::enable_shared_from_this<TokenBucket::Impl> {
	public:
		explicit Impl(uint32_t rate);
		Impl(const Impl&) = delete;
		~Impl();
		void Start();
		void Stop();
		uint32_t GetRate();
		bool GetToken(int64_t timeout);

		virtual void Run() override; // Runnable interface
		virtual void Print() {}

	private:
		using PToken = token;
		using PQueue = PriQueue<PToken>;
		using Container = Channel<PToken, PQueue>;

		uint32_t rate_;
		std::chrono::system_clock::time_point start_; // start time
		std::chrono::microseconds interval_;
		bool stop_;
		std::shared_ptr<std::thread> thread_;
		std::unique_ptr<Container> tokens_;
};

using namespace std;
using namespace std::chrono;
const uint32_t ONE_MILLION = 1000000;


TokenBucket::Impl::Impl(uint32_t rate) : 
	rate_(rate), 
	stop_(false) {
	if (rate_ == 0) {
		rate_ = 1;
	} else if (rate_ > ONE_MILLION) {
		rate_ = ONE_MILLION;
	}

	interval_ = microseconds(ONE_MILLION / rate_);
	tokens_ = std::make_unique<Container>(rate_);
}

TokenBucket::Impl::~Impl() {
	Stop();
}

void TokenBucket::Impl::Start() {
	thread_ = make_unique<std::thread>(std::bind(&TokenBucket::Impl::Run, this));
}

inline void PrintTime(const system_clock::time_point &t) {
	time_t tt = system_clock::to_time_t (t);
    std::cout << ctime(&tt) << endl;
}

void TokenBucket::Impl::Run() {
	start_ = system_clock::now();
	auto next = start_ + interval_;
	uint64_t ticks = 1;
	while( !stop_) {
		//PrintTime(next);
		std::this_thread::sleep_until(next);

		// do not wait
		token t(ticks, 1);
		//PToken elm(t, 1);
		tokens_->Put(t, -1);
		++ticks;
		next = start_ + interval_*ticks;
	}
}

void TokenBucket::Impl::Stop() {
	stop_ = true;
	tokens_->Close();
	if (thread_ != nullptr && thread_->joinable()) {
		// wait for thread to finish
		thread_->join();
	}
}

uint32_t TokenBucket::Impl::GetRate() {
	return rate_;
}

bool TokenBucket::Impl::GetToken(int64_t timeout) {
	if (stop_) {
		return false;
	}
	auto t = tokens_->Get(timeout);
	if (0 == t.GetPriority()) {
		return false;
	}

	return true;
}


TokenBucket::TokenBucket(uint32_t rate) {
	impl_ = std::make_unique<Impl>(rate);
}

TokenBucket::~TokenBucket() {}

void TokenBucket::Start() {
	impl_->Start();
}

void TokenBucket::Stop() {
	impl_->Stop();
}

uint32_t TokenBucket::GetRate() {
	return impl_->GetRate();
}

bool TokenBucket::GetToken(int64_t timeout) {
	return impl_->GetToken(timeout);
}


