workspace "TakeCEngine"
    architecture "x64"
    startproject "TakeCEngine"
    configurations { "Debug", "Release" }
    location "build"

    -- 出力先の規定化
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- 共通設定
    flags { "MultiProcessorCompile" }
    characterset "MBCS" -- 変更可: "Unicode"
    warnings "High"
    cppdialect "C++20"
    staticruntime "On" -- ランタイムを静的リンク。必要に応じて Off に

    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        runtime "Release"

    filter {}

    -- 共通のユーティリティ（vpaths など）
    dofile "scripts_premake_util.lua"

-- ソリューション内のグループ（Solution フォルダ）を分けたい場合
group "Engine"
project "TakeCEngine"
    kind "ConsoleApp"        -- エンジンをライブラリにするなら: "StaticLib" or "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- ここに実際のソース配置に合わせてパターンを追加/調整してください
    files {
        -- C/C++
        "src/**.c", "src/**.cc", "src/**.cpp",
        "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl",
        "include/**.h", "include/**.hpp", "include/**.inl",

        -- HLSL
        "shaders/**.hlsl",
        "assets/shaders/**.hlsl",

        -- その他（設定・スクリプト・リソース等）
        "scripts/**",
        "resources/**",
        "assets/**",

        -- ルートにある設定ファイルなども VS で見えるように
        ".editorconfig",
        ".clang-format",
        "CMakeLists.txt"
    }

    -- インクルードパス
    includedirs {
        "include",
        "src",
        "third_party",
        "external",
        "vendor"
    }

    -- VS のフィルタ整理（順序が重要）
    vpaths(build_external_vpaths())
    vpaths(build_default_vpaths())

    -- 必要に応じてWindows専用の設定
    filter { "system:windows" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    filter {}

group "" -- グループ解除（以降の project はルートに）