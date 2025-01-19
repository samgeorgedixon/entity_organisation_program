#include "entity_organisation_program.h"

struct Params {
	std::string filePath;
	std::string identifiers;

	int repeats;
};

Params ExtractParams(int argc, char* argv[]) {
	Params params = { "", "", 25 };
	
	if (argc > 1) {
		params.filePath = argv[1];
	}
	if (argc > 2) {
		params.identifiers = argv[2];
	}
	if (argc > 3) {
		params.repeats = std::stoi(argv[3]);
	}

	return params;
}

int main(int argc, char* argv[]) {
	Params params = ExtractParams(argc, argv);

	eop::EOP_Config eop_config = eop::ImportEOP_ConfigXLSX(params.filePath);

	if (eop_config.district.rows == 0 && eop_config.district.cols == 0) {
		return 1;
	}
	
	eop::EvaluateEOP_Config(eop_config, params.repeats);
	
	eop::PrintEOP_Config(eop_config, params.identifiers);
	int outRes = eop::ExportEOP_ConfigXLSX(params.filePath, eop_config, params.identifiers);
	
	if (!outRes) {
		return 1;
	}
    
	return 0;
}
