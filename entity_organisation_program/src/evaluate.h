#pragma once
#include "core.h"

#include "eop_config.h"

/*

- Entity Oranisation Program

- Recursion uses a lot of stack when using high row/column/entity counts so set "Stack Reserve Size" to 50,000,000.

- i = (y * cols) + x

*/

namespace eop {

	void EvaluateEOP_Config(EOP_Config& eop_config, int repeats);

}
