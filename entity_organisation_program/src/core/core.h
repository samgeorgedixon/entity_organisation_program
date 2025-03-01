#pragma once

#include "std_incl.h"
#include "misc.h"

#ifdef EOP_DISABLE_LOGGING
	#define EOP_LOG(x)
	#define EOP_TIME(x)
	#define EOP_TIME_END(x, msg)
#else
	#define EOP_LOG(x) std::cout << x
	#define EOP_TIME(x) std::chrono::steady_clock::time_point x = std::chrono::high_resolution_clock::now()
	#define EOP_TIME_END(x, msg) EOP_LOG("Time - " << msg << ": " << (float)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - x).count() << " us \n");
#endif
