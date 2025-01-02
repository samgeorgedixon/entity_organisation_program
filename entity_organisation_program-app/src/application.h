#pragma once
#include <thread>

#include "entity_organisation_program.h"

#include "sdl/SDL.h"
#include "sdl/SDL_syswm.h"
#include "imgui/imgui.h"

#include "imgui/imgui_manager.h"

#include "windows.h"
#include "commdlg.h"

namespace app {

	void Setup();
	void Run();
	void Close();

}
