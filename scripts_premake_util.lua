-- Visual Studio のフィルタ（仮想パス）をきれいにするためのヘルパ

-- 外部依存（third_party/vendor/external）を最優先で「External/*」に寄せる
function build_external_vpaths()
    return {
        -- 外部コード/ヘッダ/スクリプト/その他を丸ごと External 配下へ
        ["External/*"] = {
            "third_party/**",
            "external/**",
            "vendor/**"
        }
    }
end

-- 一般的なヘッダ/ソース/シェーダ/スクリプト/リソースの整理
function build_default_vpaths()
    return {
        -- それぞれ /* としておくと、実ディレクトリ構造を保ったまま
        -- Headers/src 下に仮想パスがぶら下がります

        ["Header Files/*"] = {
            "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl",
            "include/**.h", "include/**.hpp", "include/**.inl"
        },

        ["Source Files/*"] = {
            "src/**.c", "src/**.cc", "src/**.cpp"
        },

        ["Shaders/*"] = {
            "shaders/**.hlsl",
            "assets/shaders/**.hlsl"
        },

        ["Scripts/*"] = {
            "scripts/**"
        },

        ["Resources/*"] = {
            "resources/**",
            "assets/**"
        },

        -- ルートの設定ファイル等
        ["Config/*"] = {
            ".editorconfig",
            ".clang-format",
            "CMakeLists.txt"
        }
    }
end