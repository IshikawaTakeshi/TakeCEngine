local defaultRepositoryRoot = path.getabsolute(path.join(_SCRIPT_DIR, ".."))

local function ResolvePaths(repositoryRoot)
    local root = path.getabsolute(repositoryRoot or defaultRepositoryRoot)
    local projectRoot = path.join(root, "project")
    return {
        repositoryRoot = root,
        projectRoot = projectRoot,
        engineRoot = path.join(projectRoot, "engine"),
        externalsRoot = path.join(projectRoot, "externals"),
        packagesRoot = path.join(projectRoot, "packages"),
        contentRoot = path.join(projectRoot, "EngineContent")
    }
end

function TakeCEngineIncludeDirs(repositoryRoot)
    local paths = ResolvePaths(repositoryRoot)
    return {
        paths.projectRoot,
        paths.engineRoot,
        path.join(paths.engineRoot, "2d"),
        path.join(paths.engineRoot, "3d"),
        path.join(paths.engineRoot, "audio"),
        path.join(paths.engineRoot, "base"),
        path.join(paths.engineRoot, "io"),
        path.join(paths.engineRoot, "Scene"),
        path.join(paths.engineRoot, "math"),
        path.join(paths.engineRoot, "camera"),
        paths.externalsRoot,
        path.join(paths.externalsRoot, "assimp/include"),
        path.join(paths.externalsRoot, "DirectXTex"),
        path.join(paths.externalsRoot, "imgui"),
        path.join(paths.externalsRoot, "ImGuizmo"),
        path.join(paths.externalsRoot, "ImNodeFlow-1.2.2/include"),
        path.join(paths.externalsRoot, "nlohmann"),
        path.join(paths.externalsRoot, "magic_enum"),
        paths.packagesRoot,
        path.join(paths.packagesRoot, "Microsoft.AI.DirectML.1.15.4/include"),
        path.join(paths.packagesRoot, "Microsoft.Direct3D.DXC.1.9.2602.24/build/native/include"),
        path.join(paths.packagesRoot, "Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include")
    }
end

function DefineTakeCEngineProject(options)
    options = options or {}
    local paths = ResolvePaths(options.repositoryRoot)

    project (options.projectName or "TakeCEngine")
        location (options.projectLocation or paths.projectRoot)
        kind "StaticLib"
        targetdir (options.targetDir or path.join(paths.projectRoot, "bin/%{cfg.buildcfg}"))
        objdir (options.objectDir or path.join(paths.projectRoot, "obj/%{prj.name}/%{cfg.buildcfg}"))
        includedirs (TakeCEngineIncludeDirs(paths.repositoryRoot))

        files {
            path.join(paths.engineRoot, "**.h"),
            path.join(paths.engineRoot, "**.hpp"),
            path.join(paths.engineRoot, "**.cpp"),
            path.join(paths.contentRoot, "shaders/**.hlsl"),
            path.join(paths.externalsRoot, "imgui/**.h"),
            path.join(paths.externalsRoot, "imgui/**.cpp"),
            path.join(paths.externalsRoot, "ImGuizmo/**.h"),
            path.join(paths.externalsRoot, "ImGuizmo/**.cpp"),
            path.join(paths.externalsRoot, "ImNodeFlow-1.2.2/**.h"),
            path.join(paths.externalsRoot, "ImNodeFlow-1.2.2/**.inl"),
            path.join(paths.externalsRoot, "ImNodeFlow-1.2.2/**.cpp"),
            path.join(paths.externalsRoot, "DirectXTex/**.h"),
            path.join(paths.externalsRoot, "DirectXTex/**.cpp")
        }

        removefiles {
            path.join(paths.externalsRoot, "DirectXTex/BCDirectCompute.cpp"),
            path.join(paths.externalsRoot, "DirectXTex/DirectXTexCompressGPU.cpp")
        }

        vpaths {
            ["Engine/*"] = { path.join(paths.engineRoot, "**") },
            ["Engine/Scene/*"] = { path.join(paths.engineRoot, "Scene/**") },
            ["Shaders/*"] = { path.join(paths.contentRoot, "shaders/**") },
            ["External/imgui/*"] = { path.join(paths.externalsRoot, "imgui/**") },
            ["External/ImGuizmo/**"] = {path.join(paths.externalsRoot, "ImGuizmo/**") },
            ["External/ImNodeFlow/*"] = { path.join(paths.externalsRoot, "ImNodeFlow-1.2.2/**") },
            ["External/DirectXTex/*"] = { path.join(paths.externalsRoot, "DirectXTex/**") }
        }

        filter "files:**.hlsl"
            buildaction "None"

        filter { "files:**/externals/**.cpp" }
            warnings "Off"

        filter {}

    return paths
end

function ConfigureTakeCEngineConsumer(options)
    options = options or {}
    local paths = ResolvePaths(options.repositoryRoot)

    includedirs (TakeCEngineIncludeDirs(paths.repositoryRoot))
    dependson { options.projectName or "TakeCEngine" }
    links {
        options.projectName or "TakeCEngine",
        "DirectML",
        "onnxruntime",
        "mfplat",
        "mf",
        "mfreadwrite",
        "mfuuid",
        "dxguid"
    }

    libdirs {
        path.join(paths.packagesRoot, "Microsoft.AI.DirectML.1.15.4/bin/x64-win"),
        path.join(paths.packagesRoot, "Microsoft.Direct3D.DXC.1.9.2602.24/build/native/lib/x64"),
        path.join(paths.packagesRoot, "Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native"),
        path.join(paths.externalsRoot, "assimp/lib/Debug"),
        path.join(paths.externalsRoot, "assimp/lib/Release")
    }

    filter "configurations:Debug"
        links { "assimp-vc143-mtd" }

    filter "configurations:Develop or Release"
        links { "assimp-vc143-mt" }

    filter {}

    postbuildcommands {
        '{COPYFILE} "' .. path.join(paths.packagesRoot, 'Microsoft.AI.DirectML.1.15.4/bin/x64-win/DirectML.dll') .. '" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. path.join(paths.packagesRoot, 'Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native/onnxruntime.dll') .. '" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. path.join(paths.packagesRoot, 'Microsoft.ML.OnnxRuntime.DirectML.1.24.4/runtimes/win-x64/native/onnxruntime_providers_shared.dll') .. '" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. path.join(paths.packagesRoot, 'Microsoft.Direct3D.DXC.1.9.2602.24/build/native/bin/x64/dxcompiler.dll') .. '" "%{cfg.targetdir}"',
        '{COPYFILE} "' .. path.join(paths.packagesRoot, 'Microsoft.Direct3D.DXC.1.9.2602.24/build/native/bin/x64/dxil.dll') .. '" "%{cfg.targetdir}"'
    }

    return paths
end
