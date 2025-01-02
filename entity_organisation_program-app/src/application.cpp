#include "application.h"

namespace app {

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	bool finished = false;
	bool showImGui = true;

	int widthPixels = 400;
	int heightPixels = 245;

	std::string outputLine0 = "";
	std::string outputLine1 = "";

	int bufferSize = 200;
	char* configPathBuffer = new char[bufferSize];
	char* configPathBufferCheck = new char[bufferSize];

	char* identifierBuffer = new char[bufferSize];

	int repeatsSlider = 25;

	bool runConfigFinished = true;

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
		runConfigFinished = false;

		eop::EOP_Config eop_config = eop::LoadXMLFile(configPathBuffer);
		if (eop_config.district.rows == 0 && eop_config.district.cols == 0) {
			outputLine1 = "Unable to Open File: " + std::string(configPathBuffer);
			
			runConfigFinished = true; return;
		}

		eop::GenerateDistrict(eop_config, repeatsSlider);

		eop::PrintDistrictIteration(eop_config, 0, 1);
		int outRes = eop::WriteXML_EOPConfig(configPathBuffer, eop_config, identifierBuffer);

		if (!outRes) {
			outputLine1 = "Unable to Write File: " + std::string(configPathBuffer);
			LOG(outputLine1 << "\n");

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
		identifierBuffer[0] = 'I';
		identifierBuffer[1] = 'D';
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
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New");
				ImGui::MenuItem("Open");

				ImGui::Separator();

				ImGui::MenuItem("Preferences");

				ImGui::Separator();

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

	void Render() {
		StartFrame();

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
			std::string filePath = OpenFileDialog("XML Spreadsheet(*.xml)\0 * .xml\0").c_str();

			if (filePath.size() <= bufferSize) {
				for (int i = 0; i < filePath.size(); i++)
					configPathBuffer[i] = filePath[i];
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
			ImGui::Text("Repeats: "); ImGui::SameLine();
			ImGui::SliderInt("##repeats", &repeatsSlider, 1, 250);
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

			std::thread runConfig(RunConfig);

			while (!runConfigFinished) {
				Render();
			}
			runConfig.join();
		}

		ImGui::End();

		EndFrame();
	}

	void Run() {
		while (!finished) {
			Update();
			Render();
		}
	}

}
