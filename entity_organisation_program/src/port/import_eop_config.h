#pragma once
#include "core/core.h"

#include "evaluate/eop_config.h"
#include "evaluate/evaluate.h"

#include "port/export_eop_config.h"

namespace eop {

	EOP_Config ImportEOP_ConfigXLSX(std::string filePath);

}
