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
includes["lua"] = "entity_organisation_program/vendor/lua/include"
includes["sol2"] = "entity_organisation_program/vendor/sol2/include"

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
        "%{includes.sdl}"
    }
    libdirs {
        "%{prj.location}/vendor/sdl/lib/x64",
        "entity_organisation_program/vendor/OpenXLSX/build/lib",
        "entity_organisation_program/vendor/lua"
    }
    links {
        "entity_organisation_program",
        "imgui",
        "sdl2.lib",
        "sdl2main.lib",
        "lua54.lib",
        "comctl32.lib"
    }

    postbuildcommands {
		("{COPY} %{prj.location}vendor/sdl/lib/x64/SDL2.dll %{wks.location}bin/%{prj.name}/" .. builddir),
		("{COPY} %{wks.location}entity_organisation_program/vendor/lua/lua54.dll %{wks.location}bin/%{prj.name}/" .. builddir),
        
		("{COPY} %{wks.location}lua_modules/eop.lua %{wks.location}bin/%{prj.name}/" .. builddir),
		("{COPY} %{wks.location}lua_modules/eop.lua %{wks.location}%{prj.name}/")
	}

    filter "configurations:debug"
        kind "ConsoleApp"
        links "OpenXLSXd.lib"
    filter "configurations:release"
        kind "WindowedApp"
        defines "EOP_DISABLE_LOGGING"
        links "OpenXLSX.lib"

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
        "%{includes.OpenXLSX}",
        "%{includes.lua}",
        "%{includes.sol2}"
    }
    filter "configurations:release"
        defines "EOP_DISABLE_LOGGING"
