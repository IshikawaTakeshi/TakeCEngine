[![DebugBuild](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/DebugBuild.yml)
[![ReleaseBuild](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/ReleaseBuild.yml)

# TakeCEngine

DirectX 12を使用するゲームエンジンです。ゲーム固有コードとゲーム資産は
[Face_Game](https://github.com/IshikawaTakeshi/Face_Game)へ分離されています。

## 構成

```text
TakeCEngine/
├─ project/
│  ├─ engine/          # エンジンソース
│  ├─ EngineContent/   # シェーダー、フォント、既定画像
│  ├─ externals/       # エンジンの外部依存
│  └─ packages.config
├─ premake/
└─ tools/
```

`project/engine`からゲームアプリケーションへの直接依存はありません。
`tools/check_engine_boundary.py`がこの境界を検査し、CIでも実行します。

## プロジェクト生成とビルド

Visual Studio 2022／PlatformToolset v143:

```bat
nuget restore project\packages.config -PackagesDirectory project\packages
premake\GenerateProject.bat v143
msbuild project\TakeCEngine.sln /p:Platform=x64 /p:Configuration=Debug /p:PlatformToolset=v143 /m
```

Visual Studio 2026を使う場合は、`v143`を`v145`へ変更します。

構成は`Debug`、`Develop`、`Release`の3種類です。

## ゲームから利用する

ゲームリポジトリへsubmoduleとして追加します。

```bat
git submodule add https://github.com/IshikawaTakeshi/TakeCEngine vendor\TakeCEngine
```

ゲーム側Premakeから共通定義を読み込みます。

```lua
local engineRepository = "vendor/TakeCEngine"
dofile(path.join(engineRepository, "premake/TakeCEngineProject.lua"))

DefineTakeCEngineProject {
    repositoryRoot = engineRepository,
    projectLocation = "project"
}

project "Game"
    ConfigureTakeCEngineConsumer {
        repositoryRoot = engineRepository
    }
```

ゲーム開始時にゲーム資産とエンジン資産のルートを指定します。

```cpp
TakeC::ResourceRootConfig resources{
    .gameRoot = "Assets",
    .engineRoot = "../vendor/TakeCEngine/project/EngineContent",
};
game->Run(L"Game", resources);
```
