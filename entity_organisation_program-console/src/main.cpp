#include "entity_organisation_program.h"

int main(int argc, char* argv[]) {
	std::string filePath = "../spreadsheets/eop_config.xml";
	if (argc > 1)
		filePath = argv[1];

	eop::EOP_Config eop_config = eop::LoadXMLFile(filePath);
	 
	eop::GenerateDistrict(eop_config, 25);
	
	eop::PrintDistrictIteration(eop_config, 0, 1);
    return 0;
}
