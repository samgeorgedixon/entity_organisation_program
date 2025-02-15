#pragma once

#include "std_incl.h"
#include "misc.h"

#ifdef EOP_DISABLE_LOGGING
	#define EOP_LOG(x)
#else
	#define EOP_LOG(x) std::cout << x
#endif
