project "imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

    targetdir (bin .. "imgui/" .. build)
    objdir (bin_int .. "imgui/" .. build)

	files {
		"imgui/imconfig.h",
		"imgui/imgui.h",
		"imgui/imgui.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_internal.h",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h",
		"imgui/imgui_demo.cpp",
		"imgui/imgui_tables.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
	filter "configurations:debug"
		symbols "on"
	filter "configurations:release"
		optimize "on"
