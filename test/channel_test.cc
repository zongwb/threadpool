#include <iostream>

#include "priqueue.h"
#include "channel.h"

using namespace std;

struct Item {
	Item() {}
	Item(int p) : priority_(p) {}
	Priority priority_;
	int GetPriority() {
		return priority_.GetPriority();
	}
};

namespace std{
template<>
class less<Item> {
	public:
		bool operator() (const Item& x, const Item& y) const {
			return std::less<Priority>()(x.priority_, y.priority_);
		}
};
}

int main() {
	int N = 10;
	Channel<Item, FifoQueue<Item>> chan(N);
	for (int i = 0; i < N; ++i) {
		auto itm = Item(i);
		chan.Put(itm, -1);
	}

	for (int i = 0; i < N; ++i) {
		auto itm = chan.Get(0);
		cout << "item  " << itm.GetPriority() << endl;
	}
}
