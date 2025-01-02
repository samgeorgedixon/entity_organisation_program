#pragma once

#include "std_incl.h"

#ifdef DEBUG
	#define LOG(x) std::cout << x
	#define LOG_WARNING(x) std::cout << "WARNING: " << x
	#define LOG_ERROR(x) std::cout << "ERROR: " << x
	
	#define ASSERT(x, ...) { if (!x) { LOG_ERROR("Assert Failed: {0}", __VA_ARGS__); __debugbreak; } }
#else
	#define LOG(x)
	#define LOG_WARNING(x)
	#define LOG_ERROR(x)
	#define ASSERT(x, ...)
#endif
