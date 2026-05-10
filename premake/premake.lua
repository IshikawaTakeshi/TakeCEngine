workspace "TakeCEngine"
    architecture "x64"
    startproject "DirectXGame"

    -- .sln / .vcxproj の出力先を project/ フォルダに合わせる
    -- (このスクリプトは premake/ にあるので ../project が正しい)
    location "../project"

    -- ソリューションファイル名を既存に合わせる（デフォルトは "TakeCEngine.sln" になる）
    filename "DirectXGame"

    configurations { "Debug", "Release" }

    -- 出力先ディレクトリの設定
    -- .vcxproj が project/ にあるので、そこからの相対パスになる
    targetdir "../generated/outputs/%{cfg.buildcfg}"
    objdir    "../generated/obj/%{prj.name}/%{cfg.buildcfg}"

    -- 基本的な定義
    defines { "_UNICODE", "UNICODE", "NOMINMAX" }

    -- C++標準バージョン
    cppdialect "C++20"

-- -----------------------------------------------------------------------
-- プロジェクト定義
-- -----------------------------------------------------------------------
project "DirectXGame"
    kind "WindowedApp"
    language "C++"

    -- -----------------------------------------------------------------------
    -- ソースファイルの指定
    -- externals はコンパイルが必要な CPP ファイルのみ明示的に列挙する。
    -- externals のヘッダはすべて includedirs で解決するため files に含めない。
    -- -----------------------------------------------------------------------

    -- [整理対象] engine / application / Resources / ルートファイル
    files {
        "../project/engine/**.cpp",
        "../project/engine/**.h",
        "../project/application/**.cpp",
        "../project/application/**.h",
        "../project/Resources/shaders/**.hlsl",
        "../project/Resources/shaders/**.hlsli",
        "../project/main.cpp",
        "../project/resource.h",
        "../project/DirectXGame.rc",
    }

    -- [整理対象外] externals: コンパイルが必要な CPP のみ
    files {
        -- ImGui
        "../project/externals/imgui/imgui.cpp",
        "../project/externals/imgui/imgui_demo.cpp",
        "../project/externals/imgui/imgui_draw.cpp",
        "../project/externals/imgui/imgui_impl_dx12.cpp",
        "../project/externals/imgui/imgui_impl_win32.cpp",
        "../project/externals/imgui/imgui_tables.cpp",
        "../project/externals/imgui/imgui_widgets.cpp",
        -- ImNodeFlow
        "../project/externals/ImNodeFlow-1.2.2/src/ImNodeFlow.cpp",
        -- DirectXTex (ソースをまるごとコンパイル)
        "../project/externals/DirectXTex/**.cpp",
    }

    -- .hlsl はVSに表示するだけでビルドから除外する（独自DXCビルドを使用するため）
    filter { "files:**.hlsl" }
        buildaction "None"
    filter {}

    -- インクルードパスの設定
    includedirs {
        "../project",
        "../project/engine",
        "../project/application",
        "../project/externals/assimp/include",
        "../project/externals/DirectXTex",
        "../project/externals/imgui",
        "../project/externals/nlohmann",
        "../project/externals/magic_enum",
        "../project/Resources/shaders",
        "../project/externals/ImNodeFlow-1.2.2/src",
    }

    -- -----------------------------------------------------------------------
    -- VSフィルタの自動整理 (vpaths)
    --   整理対象: engine / application / Resources / ルートファイル
    --     → "../project/" を除いた残りのパスをそのままフィルタ階層にする
    --       例: ../project/engine/3d/Foo.cpp  → フィルタ engine\3d
    --   整理対象外: externals
    --     → vpaths パターンに合わせず、"externals\..." パスのまま扱う
    -- -----------------------------------------------------------------------
    vpaths {
        ["engine/**"]      = "../project/engine/**",
        ["application/**"] = "../project/application/**",
        ["Resources/**"]   = "../project/Resources/**",
        ["**"]             = { "../project/main.cpp", "../project/resource.h", "../project/DirectXGame.rc" },
    }

    -- Windows 用の設定
    filter "system:windows"
        systemversion "latest"

    -- Debug 設定
    filter "configurations:Debug"
        defines  { "_DEBUG" }
        runtime  "Debug"
        symbols  "On"
        libdirs  { "../project/externals/assimp/lib/Debug" }
        links    { "assimp-vc143-mtd.lib" }

    -- Release 設定
    filter "configurations:Release"
        defines  { "NDEBUG" }
        runtime  "Release"
        optimize "On"
        libdirs  { "../project/externals/assimp/lib/Release" }
        links    { "assimp-vc143-mt.lib" }

    -- ビルド後のイベント（filter {} でリセットしてから記述する）
    filter {}
    postbuildcommands {
        "{COPY} \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll\" \"%{cfg.targetdir}\"",
        "{COPY} \"$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll\" \"%{cfg.targetdir}\"",
    }