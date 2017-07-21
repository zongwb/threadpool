#ifndef __RATELIMITER_H_
#define __RATELIMITER_H_

#include <stdint.h>

class RateLimiter {
	public:
		virtual bool GetToken(int64_t timeout) = 0;
		virtual uint32_t GetRate() = 0; // In terms of tokens per second.
		virtual void Start() = 0;
		virtual void Stop() = 0;
};

#endif // __RATELIMITER_H_
