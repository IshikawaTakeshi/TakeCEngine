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

local projectRoot = "../project"
local engineRoot = projectRoot .. "/engine"
local applicationRoot = projectRoot .. "/application"
local externalsRoot = projectRoot .. "/externals"
local packagesRoot = projectRoot .. "/packages"

local commonIncludeDirs = {
    projectRoot,
    engineRoot,
    engineRoot .. "/2d",
    engineRoot .. "/3d",
    engineRoot .. "/audio",
    engineRoot .. "/base",
    engineRoot .. "/io",
    engineRoot .. "/scene",
    engineRoot .. "/math",
    engineRoot .. "/camera",
    applicationRoot,
    externalsRoot,
    externalsRoot .. "/assimp/include",
    externalsRoot .. "/DirectXTex",
    externalsRoot .. "/imgui",
    externalsRoot .. "/nlohmann",
    externalsRoot .. "/magic_enum",
    externalsRoot .. "/ImNodeFlow-1.2.2/include",
    packagesRoot,
    packagesRoot .. "/Microsoft.AI.DirectML.1.15.4/include",
    packagesRoot .. "/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include"
}

project "TakeCEngine"
    location (projectRoot)
    kind "StaticLib"
    targetdir (projectRoot .. "/bin/%{cfg.buildcfg}")
    objdir (projectRoot .. "/obj/%{prj.name}/%{cfg.buildcfg}")
    includedirs (commonIncludeDirs)

    files {
        engineRoot .. "/**.h",
        engineRoot .. "/**.hpp",
        engineRoot .. "/**.cpp",
        projectRoot .. "/Resources/shaders/**.hlsl",
        externalsRoot .. "/imgui/**.h",
        externalsRoot .. "/imgui/**.cpp",
        externalsRoot .. "/DirectXTex/**.h",
        externalsRoot .. "/DirectXTex/**.cpp",

        -- These scene abstractions are currently required by TakeCFrameWork.
        -- They remain here during the transition and can be moved into engine later.
        applicationRoot .. "/Scene/AbstractSceneFactory.h",
        applicationRoot .. "/Scene/BaseScene.h",
        applicationRoot .. "/Scene/LevelData.h",
        applicationRoot .. "/Scene/SceneManager.h",
        applicationRoot .. "/Scene/SceneManager.cpp",
        applicationRoot .. "/Scene/SceneTransition.h",
        applicationRoot .. "/Scene/SceneTransition.cpp"
    }

    -- GPU compression sources require generated shader .inc files that are not
    -- included in this repository. CPU texture loading remains available.
    removefiles {
        externalsRoot .. "/DirectXTex/BCDirectCompute.cpp",
        externalsRoot .. "/DirectXTex/DirectXTexCompressGPU.cpp"
    }

    vpaths {
        ["Engine/*"] = { engineRoot .. "/**" },
        ["Engine/Scene/*"] = { applicationRoot .. "/Scene/**" },
        ["Shaders/*"] = { projectRoot .. "/Resources/shaders/**" },
        ["External/imgui/*"] = { externalsRoot .. "/imgui/**" },
        ["External/DirectXTex/*"] = { externalsRoot .. "/DirectXTex/**" }
    }

    filter "files:**.hlsl"
        buildaction "None"

    filter { "files:../project/externals/**.cpp" }
        warnings "Off"

    filter {}

project "DirectXGame"
    location (projectRoot)
    kind "WindowedApp"
    targetdir (projectRoot .. "/bin/%{cfg.buildcfg}")
    objdir (projectRoot .. "/obj/%{prj.name}/%{cfg.buildcfg}")
    includedirs (commonIncludeDirs)
    dependson { "TakeCEngine" }

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

    removefiles {
        applicationRoot .. "/Scene/SceneManager.cpp",
        applicationRoot .. "/Scene/SceneTransition.cpp"
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

    links {
        "TakeCEngine",
        "DirectML",
        "onnxruntime",
        "mfplat",
        "mf",
        "mfreadwrite",
        "mfuuid",
        "dxguid"
    }

    libdirs {
        packagesRoot .. "/Microsoft.AI.DirectML.1.15.4/bin/x64-win",
        packagesRoot .. "/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native",
        externalsRoot .. "/assimp/lib/Debug",
        externalsRoot .. "/assimp/lib/Release"
    }

    filter "configurations:Debug"
        links { "assimp-vc143-mtd" }

    filter "configurations:Develop"
        links { "assimp-vc143-mt" }

    filter "configurations:Release"
        links { "assimp-vc143-mt" }

    filter {}

    postbuildcommands {
        '{COPYFILE} "' .. packagesRoot .. '/Microsoft.AI.DirectML.1.15.4/bin/x64-win/DirectML.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. packagesRoot .. '/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native/onnxruntime.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. packagesRoot .. '/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native/onnxruntime_providers_shared.dll" "%{cfg.targetdir}"'
    }
