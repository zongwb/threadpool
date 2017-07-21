#ifndef __TOKENBUCKET_H_
#define __TOKENBUCKET_H_

#include "ratelimiter.h"

#include <memory>

// TokenBucket implements a token bucket algorithm according to the  RateLimiter interface.
class TokenBucket : public RateLimiter {
	public:
		explicit TokenBucket(uint32_t rate);
		TokenBucket(const TokenBucket&) = delete;
		TokenBucket(TokenBucket&&) = delete;
		TokenBucket& operator+(const TokenBucket&) = delete;
		TokenBucket& operator+(TokenBucket&&) = delete;
		~TokenBucket();
		
		virtual void Start() override;
		virtual void Stop() override;
		virtual uint32_t GetRate() override;

		// GetToken may be blocking or nonblocking, depending on the value of timeout.
		// If timeout == 0, it returns immediately indicating if a token is obtained.
		// If timeout < 0, it blocks until a token is obtained.
		// If timeout > 0, it waits for a maximum of timeout milliseconds to obtain a token.
		virtual bool GetToken(int64_t timeout = -1) override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;
};

#endif // __TOKENBUCKET_H_
