workspace "TakeCEngine"
    filename "DirectXGame"
    location "../project"
    architecture "x86_64"
    configurations { "Debug", "Develop", "Release" }
    startproject "DirectXGame"

    language "C++"
    cppdialect "C++20"
    characterset "Unicode"
    staticruntime "On"
    warnings "Extra"
    fatalwarnings "All"
    defines { "NOMINMAX" }
    multiprocessorcompile "On"
    buildoptions { "/utf-8", "/FIWindows.h" }

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Develop"
        defines { "_DEVELOP" }
        runtime "Release"
        symbols "On"
        optimize "Speed"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"

    filter {}

local repositoryRoot = ".."
local projectRoot = "../project"
local applicationRoot = projectRoot .. "/application"
local externalsRoot = projectRoot .. "/externals"

dofile(path.join(_SCRIPT_DIR, "TakeCEngineProject.lua"))
DefineTakeCEngineProject {
    repositoryRoot = repositoryRoot,
    projectLocation = projectRoot,
    targetDir = projectRoot .. "/bin/%{cfg.buildcfg}",
    objectDir = projectRoot .. "/obj/%{prj.name}/%{cfg.buildcfg}"
}

project "DirectXGame"
    location (projectRoot)
    kind "WindowedApp"
    targetdir (projectRoot .. "/bin/%{cfg.buildcfg}")
    objdir (projectRoot .. "/obj/%{prj.name}/%{cfg.buildcfg}")
    debugdir (projectRoot)
    includedirs {
        applicationRoot,
        externalsRoot .. "/ImNodeFlow-1.2.2/include"
    }

    files {
        applicationRoot .. "/**.h",
        applicationRoot .. "/**.hpp",
        applicationRoot .. "/**.cpp",
        externalsRoot .. "/ImNodeFlow-1.2.2/include/**.h",
        externalsRoot .. "/ImNodeFlow-1.2.2/include/**.hpp",
        externalsRoot .. "/ImNodeFlow-1.2.2/src/**.h",
        externalsRoot .. "/ImNodeFlow-1.2.2/src/**.inl",
        externalsRoot .. "/ImNodeFlow-1.2.2/src/**.cpp",
        projectRoot .. "/main.cpp",
        projectRoot .. "/resource.h",
        projectRoot .. "/DirectXGame.rc",
        projectRoot .. "/packages.config"
    }

    vpaths {
        ["Application/*"] = { applicationRoot .. "/**" },
        ["External/ImNodeFlow/*"] = { externalsRoot .. "/ImNodeFlow-1.2.2/**" },
        ["Source/*"] = {
            projectRoot .. "/main.cpp",
            projectRoot .. "/resource.h",
            projectRoot .. "/DirectXGame.rc",
            projectRoot .. "/packages.config"
        }
    }

    ConfigureTakeCEngineConsumer { repositoryRoot = repositoryRoot }
