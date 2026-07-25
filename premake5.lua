workspace "entity_organisation_program"
    architecture "x64"
    language "C++"
    cppdialect "C++17"

    startproject "entity_organisation_program"
    
    configurations { "debug", "release" }

    filter "system:windows"
        systemversion "latest"
        defines "PLATFORM_WINDOWS"
        staticruntime "on"
        entrypoint "mainCRTStartup"
    filter "configurations:debug"
        defines "DEBUG"
        symbols "on"
    filter "configurations:release"
        defines "RELEASE"
        optimize "on"

build = "%{cfg.buildcfg}-%{cfg.system}/"
bin = "%{wks.location}/bin/"
bin_int = "%{wks.location}/bin/bin-int/"

includes = {}
includes["imgui"] = "vendor/imgui"
includes["sdl3"] = "vendor/sdl3/include"

includes["openxlsx"] = "vendor/openxlsx/build/output/include"
includes["lua54"] = "vendor/lua54/include"
includes["sol2"] = "vendor/sol2/include"

group "dependencies"
    include "vendor/imgui"
group ""

project "app"
    location "app"
    
    targetname "entity_organisation_program"
    targetdir (bin .. "app/" .. build)
    objdir (bin_int .. "app/" .. build)
    
    files {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h"
    }
    includedirs {
        "%{prj.location}/src",
        "engine/src",
        
        "%{includes.imgui}",
        "%{includes.sdl3}"
    }
    libdirs {
        "vendor/sdl3/lib/x64",
        
        "vendor/lua54",
    }
    links {
        "engine",
        
        "imgui",
        "SDL3.lib",
        
        "lua54.lib",
        
        "miniz.lib",
        "nowide.lib",
        "pugixml.lib"
    }
    
    postbuildcommands {
        ("{COPY} %{wks.location}vendor/sdl3/lib/x64/SDL3.dll %{cfg.targetdir}"),
        
		("{COPY} %{wks.location}examples/lua_modules/eop.lua %{cfg.targetdir}"),
		("{COPY} %{wks.location}examples/lua_modules/eop.lua %{prj.location}")
	}
    
    filter "configurations:debug"
        kind "ConsoleApp"
        libdirs {
            "vendor/openxlsx/build-debug/output/lib",
            "vendor/openxlsx/build-debug/output/lib/OpenXLSX"
        }
        links "OpenXLSXd.lib"
    filter "configurations:release"
        kind "WindowedApp"
        libdirs {
            "vendor/openxlsx/build-release/output/lib",
            "vendor/openxlsx/build-release/output/lib/OpenXLSX"
        }
        links "OpenXLSX.lib"
        defines "EOP_DISABLE_LOGGING"
    
project "engine"
    location "engine"
    kind "StaticLib"
    
    targetname "entity_organisation_program_engine"
    targetdir (bin .. "engine/" .. build)
    objdir (bin_int .. "engine/" .. build)

    files {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h"
    } 
    includedirs {
        "%{prj.location}/src",
        
        "%{includes.lua54}",
        "%{includes.openxlsx}",
        "%{includes.sol2}"
    }
