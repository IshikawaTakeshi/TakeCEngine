workspace "TakeCEngine"
    architecture "x64"
    startproject "DirectXGame"

    -- 【重要】ソリューションファイル(.sln)を1つ上の階層(ルート)に出力する設定
    location ".."

    configurations { "Debug", "Release" }

    -- 出力先ディレクトリの設定（スクリプト位置基準なので ../ でルートに戻る）
    targetdir "../generated/outputs/%{cfg.buildcfg}"
    objdir "../generated/obj/%{prj.name}/%{cfg.buildcfg}"

    -- 基本的な定義
    defines { "_UNICODE", "UNICODE", "NOMINMAX" }
    
    -- C++標準バージョン
    cppdialect "C++20"

    -- プロジェクト定義
    project "DirectXGame"
        kind "WindowedApp"
        language "C++"
        
        -- ソースファイルの指定（../project/ から記述する）
        files { 
            "../project/**.cpp", 
            "../project/**.h", 
            "../project/**.hlsli", 
            "../project/**.hlsl",
            "../project/**.rc",
            "../project/**.png",
        }

        -- インクルードパスの設定（すべて ../project/ から記述する）
        includedirs { 
            "../project",
            "../project/engine",
            "../project/application",
            "../project/externals/assimp/include",
            "../project/externals/DirectXTex",
            "../project/externals/imgui",
            "../project/externals/nlohmann",
            "../project/externals/magic_enum",
            "../project/Resources/shaders"
        }

        -- フィルタの自動整理設定 (vpaths)
        -- "../project" 部分をトリミングして、それ以下の階層構造をフィルタとして使用する
        vpaths {
            ["*"] = "../project"
        }

        -- Windows用の設定
        filter "system:windows"
            systemversion "latest"
            
        -- Debug設定
        filter "configurations:Debug"
            defines { "_DEBUG" }
            runtime "Debug"
            symbols "On"
            -- ライブラリリンク
            libdirs { "../project/externals/assimp/lib/Debug" }
            links { "assimp-vc143-mtd.lib" }

        -- Release設定
        filter "configurations:Release"
            defines { "NDEBUG" }
            runtime "Release"
            optimize "On"
            -- ライブラリリンク
            libdirs { "../project/externals/assimp/lib/Release" }
            links { "assimp-vc143-mt.lib" }

        -- ビルド後のイベント
        filter "{}"
        postbuildcommands {
            "{COPY} \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll\" \"%{cfg.targetdir}\"",
            "{COPY} \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll\" \"%{cfg.targetdir}\""
        }