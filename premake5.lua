workspace "entity_organisation_program"
    architecture "x64"
    language "C++"
    cppdialect "C++17"

    startproject "entity_organisation_program-app"
    
    configurations { "debug", "release" }

    filter "system:windows"
        systemversion "latest"
        defines "PLATFORM_WINDOWS"
        staticruntime "on"
    filter "configurations:debug"
        defines "DEBUG"
        symbols "on"
    filter "configurations:release"
        defines "RELEASE"
        optimize "on"

builddir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

includes = {}
includes["imgui"] = "entity_organisation_program-app/vendor/imgui"
includes["sdl"] = "entity_organisation_program-app/vendor/sdl/include"

includes["OpenXLSX"] = "entity_organisation_program/vendor/OpenXLSX/build/include"

group "dependencies"
    include "entity_organisation_program-app/vendor/imgui/imgui"
group ""

project "entity_organisation_program-app"
    location "entity_organisation_program-app"

    targetdir ("%{wks.location}/bin/%{prj.name}/" .. builddir)
    objdir ("%{wks.location}/bin_int/%{prj.name}/" .. builddir)

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }
    includedirs {
        "%{prj.name}/src",
        "entity_organisation_program/src",
        "%{includes.imgui}",
        "%{includes.sdl}",
        "%{includes.nativefiledialog}"
    }
    libdirs {
        "%{prj.location}/vendor/sdl/lib/x64",
        "entity_organisation_program/vendor/OpenXLSX/build/lib"
    }
    links {
        "entity_organisation_program",
        "imgui",
        "sdl2.lib",
        "sdl2main.lib",
        "comctl32.lib",
        "OpenXLSX.lib"
    }

    postbuildcommands {
		("{COPY} %{prj.location}vendor/sdl/lib/x64/SDL2.dll %{wks.location}bin/%{prj.name}/" .. builddir)
	}

    filter "configurations:debug"
        kind "ConsoleApp"
    filter "configurations:release"
        kind "WindowedApp"
        defines "EOP_DISABLE_LOGGING"

project "entity_organisation_program-console"
    location "entity_organisation_program-console"
    kind "ConsoleApp"

    targetdir ("%{wks.location}/bin/%{prj.name}/" .. builddir)
    objdir ("%{wks.location}/bin_int/%{prj.name}/" .. builddir)

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }
    includedirs {
        "%{prj.name}/src",
        "entity_organisation_program/src",
        "entity_organisation_program/vendor/OpenXLSX/build/lib"
    }
    links {
        "entity_organisation_program",
        "OpenXLSX.lib"
    }

project "entity_organisation_program"
    location "entity_organisation_program"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/%{prj.name}/" .. builddir)
    objdir ("%{wks.location}/bin_int/%{prj.name}/" .. builddir)

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }
    includedirs {
        "%{prj.name}/src",
        "%{includes.OpenXLSX}"
    }
