#include "random.h"

namespace eop {

	void SetupRandom() {
		srand((unsigned)time(NULL));
	}

	int RandomIntRange(int low, int high) {
		if (high == 0)
			return 0;

		return low + (rand() % high);
	}

}
