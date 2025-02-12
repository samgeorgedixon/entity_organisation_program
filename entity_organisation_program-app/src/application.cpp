#include "application.h"

namespace app {

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	bool finished = false;
	bool showImGui = true;

	int widthPixels = 400;
	int heightPixels = 222;

	std::string outputLine0 = "";
	std::string outputLine1 = "";

	int bufferSize = 200;
	char* configPathBuffer = new char[bufferSize];
	char* configPathBufferCheck = new char[bufferSize];

	char* identifierBuffer = new char[bufferSize];

	int depthSlider = 1;
	int repeatsSlider = 25;
	
	bool fullRandom = false;
	bool entitiesRandom = true;

	static bool runConfigFinished = true;

	std::string OpenFileDialog(const char* filter) {
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		HWND hwnd = wmInfo.info.win.window;

		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;

		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return "";
	}
	
	void RunConfig() {
		eop::EOP_Config eop_config = eop::ImportEOP_ConfigXLSX(configPathBuffer);
		if (eop_config.district.rows == 0 && eop_config.district.cols == 0) {
			outputLine1 = "Unable to Open File: " + std::string(configPathBuffer);
			
			runConfigFinished = true; return;
		}

		int depth = 1;
		if (fullRandom) {
			depth = repeatsSlider;
		}
		else {
			depth = depthSlider;
		}

		eop::EvaluateEOP_Config(eop_config, depth, fullRandom, entitiesRandom);
		
		eop::PrintEOP_Config(eop_config, "name");

		int outRes = eop::ExportEOP_ConfigXLSX(configPathBuffer, eop_config, identifierBuffer);
		
		if (!outRes) {
			outputLine1 = "Unable to Write File: " + std::string(configPathBuffer);
			EOP_LOG(outputLine1 << "\n");
		
			runConfigFinished = true; return;
		}

		outputLine1 = "Completed";
		runConfigFinished = true;
	}

	void Setup() {
		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(widthPixels, heightPixels, 0, &window, &renderer);

		SDL_RenderSetVSync(renderer, 1);
		SDL_RenderSetScale(renderer, 1, 1);
		SDL_SetWindowTitle(window, "Entity Organisation Program");

		ImGuiSetup(window, renderer);

		for (int i = 0; i < bufferSize; i++) {
			configPathBuffer[i]			= 0;
			configPathBufferCheck[i]	= 0;
			identifierBuffer[i]			= 0;
		}
		identifierBuffer[0] = 'n';
		identifierBuffer[1] = 'a';
		identifierBuffer[2] = 'm';
		identifierBuffer[3] = 'e';
	}
	
	void Close() {
		ImGuiEnd();

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();
	}

	void StartFrame() {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				finished = true;
			ImGui_ImplSDL2_ProcessEvent(&event);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		ImGuiStartFrame();
	}

	void EndFrame() {
		ImGuiRender(window, renderer);

		SDL_RenderPresent(renderer);
	}

	ImGuiWindowFlags SetFullscreen() {
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PopStyleVar(2);
		return window_flags;
	}

	void Menu() {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Exit", NULL, false, finished != true))
					finished = true;
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void CheckConfigPathBufferChange() {
		bool changed = false;
		for (int i = 0; i < bufferSize; i++) {
			if (configPathBuffer[i] != configPathBufferCheck[i]) {
				changed = true; break;
			}
		}
		if (changed) {
			outputLine0 = "";
			outputLine1 = "";
			memcpy(configPathBufferCheck, configPathBuffer, bufferSize);
		}
	}

	void Update() {
		CheckConfigPathBufferChange();
	}

	void RenderGUI() {
		ImGui::Begin("Entity Organisation Program", &showImGui, SetFullscreen());

		Menu();

		ImGui::Text("Organise Config");
		ImGui::Spacing();

		ImGui::Separator();

		ImGui::Spacing();
		ImGui::Text("Config File: "); ImGui::SameLine();

		ImGui::PushItemWidth(widthPixels / 2);
		ImGui::InputText("##config_file", configPathBuffer, bufferSize); ImGui::SameLine();
		ImGui::PopItemWidth();

		if (ImGui::Button("Open...")) {
			std::string filePath = OpenFileDialog("Excel Workbook (*.xlsx)\0 * .xlsx\0").c_str();

			if (filePath.size() <= bufferSize) {
				int i = 0;
				for (i; i < filePath.size(); i++) {
					configPathBuffer[i] = filePath[i];
				}
				while (i < bufferSize) {
					configPathBuffer[i] = 0;
					i++;
				}
			}
		}

		ImGui::Spacing();
		ImGui::Text("Identifier:  "); ImGui::SameLine();

		ImGui::PushItemWidth(widthPixels / 2);
		ImGui::InputText("##identifier", identifierBuffer, bufferSize); ImGui::SameLine();
		ImGui::PopItemWidth();

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::Spacing();
		if (ImGui::CollapsingHeader("Advanced")) {
			ImGui::Text("Full Random:     "); ImGui::SameLine();
			ImGui::Checkbox("##fullRandom", &fullRandom);

			if (fullRandom) {
				ImGui::Text("Repeats:     "); ImGui::SameLine();
				ImGui::SliderInt("##repeats", &repeatsSlider, 1, 100);
			}
			else {
				ImGui::Text("Entities Random: "); ImGui::SameLine();
				ImGui::Checkbox("##entitiesRandom", &entitiesRandom);

				ImGui::Text("Depth:       "); ImGui::SameLine();
				ImGui::SliderInt("##depth", &depthSlider, 1, 100);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::Text("- %s", outputLine0.c_str());
		ImGui::Spacing();
		ImGui::Text("- %s", outputLine1.c_str());

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::Spacing();
		if (ImGui::Button("Run") && runConfigFinished) {
			outputLine0 = "Running...";
			outputLine1 = "";

			runConfigFinished = false;

			std::thread runConfig(RunConfig);

			ImGui::End();
			EndFrame();

			while (!runConfigFinished) {
				StartFrame();

				RenderGUI();

				EndFrame();
			}
			runConfig.join();

			return;
		}

		ImGui::End();
	}

	void Render() {
		StartFrame();

		RenderGUI();

		EndFrame();
	}

	void Run() {
		while (!finished) {
			Update();
			Render();
		}
	}

}
