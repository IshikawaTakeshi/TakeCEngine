workspace "TakeCEngine"
    filename "TakeCEngine"
    location "../project"
    architecture "x86_64"
    configurations { "Debug", "Develop", "Release" }
    startproject "TakeCEngine"

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

dofile(path.join(_SCRIPT_DIR, "TakeCEngineProject.lua"))
DefineTakeCEngineProject {
    repositoryRoot = "..",
    projectLocation = "../project",
    targetDir = "../project/bin/%{cfg.buildcfg}",
    objectDir = "../project/obj/%{prj.name}/%{cfg.buildcfg}"
}
