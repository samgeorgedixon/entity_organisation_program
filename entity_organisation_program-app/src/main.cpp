#include "application.h"

int main(int argc, char* argv[]) {
	app::Setup();
	app::Run();

	app::Close();
	return 0;
}
