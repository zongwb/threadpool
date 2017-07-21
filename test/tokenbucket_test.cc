#include <iostream>
#include <chrono>
#include <ctime>

#include "tokenbucket.h"

using namespace std;
using namespace std::chrono;

uint32_t rate = 10;

inline void PrintTime(const system_clock::time_point &t) {
	time_t tt;
	tt = system_clock::to_time_t (t);
    std::cout << ctime(&tt) << endl;
}

int main() {

	TokenBucket tb(rate);
	tb.Start();
	tb.GetToken();

	for (int i = 0; i < 10; ++i) {
		if (tb.GetToken()) {
			cout << "Got token\n";
			PrintTime(system_clock::now());
		} else {
			cout << "fail to get token\n";
		}
	}

	cout << "Exiting..." << endl;
	tb.Stop();

	return 0;
}
