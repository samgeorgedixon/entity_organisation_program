project "imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

    targetdir ("%{wks.location}/bin/%{prj.name}/" .. builddir)
    objdir ("%{wks.location}/bin_int/%{prj.name}/" .. builddir)

	files {
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",
		"imgui_tables.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"
    
	filter "configurations:debug"
		symbols "on"
	filter "configurations:release"
		optimize "on"
    filter { "system:windows", "configurations:release" }
        buildoptions "/MT"
