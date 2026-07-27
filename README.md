[![DebugBuild](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/DebugBuild.yml)
[![ReleaseBuild](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/IshikawaTakeshi/TakeCEngine/actions/workflows/ReleaseBuild.yml)

2026 07/06 書き込み
# GitHubブランチ命名規則

（元デベロップバージョン名）_(作業種別)_(作業名)

## 作業種別
- **Develop** : 大元バージョン
- **Feature** : 要素の追加
- **Refactoring** : 要素のリファクタリング
- **Engine** : エンジン更新
- **Fix** : 修正
- **Hotfix** : やばいバグ

## 例
- Ver0.0/Feature/SceneManager

---

### 現在のバージョン
- Ver1.0/顔バトル(仮)

---

## エンジンとゲームの分離

現在はリポジトリを物理分割する直前の移行用モノレポです。ディレクトリの責務は次のとおりです。

```text
project/
├─ engine/          # TakeCEngineのソース。applicationを参照しない
├─ EngineContent/   # シェーダー、フォント、エンジン既定画像
├─ application/     # A_CORE固有のゲームコード
└─ Assets/          # A_CORE固有のモデル、画像、音声、JSON、ONNX
```

SceneManager、BaseScene、SceneTransition、AbstractSceneFactoryは`engine/Scene`へ移動済みです。
ゲーム固有のLevelData、LevelObject、GameLevelはapplication側に置き、エンジンからゲームへの
逆依存をなくしています。`tools/check_engine_boundary.py`がこの境界を検査し、CIでも実行します。

### プロジェクト生成とビルド

必要なNuGetパッケージは`project/packages.config`に定義されています。構成は`Debug`、
`Develop`、`Release`の3種類です。

モノレポ全体を生成・ビルドする場合:

```bat
nuget restore project\packages.config -PackagesDirectory project\packages
premake\GenerateProject.bat v143
msbuild project\DirectXGame.sln /p:Platform=x64 /p:Configuration=Debug /p:PlatformToolset=v143 /m
```

エンジンだけを生成・ビルドする場合:

```bat
premake\GenerateEngineProject.bat v143
msbuild project\TakeCEngine.sln /p:Platform=x64 /p:Configuration=Debug /p:PlatformToolset=v143 /m
```

ローカル環境でVisual Studio 2026を使う場合は`v143`を`v145`へ変更します。

### 別ゲームから利用する

分割後のゲームリポジトリは次の構成を想定しています。

```text
NewGame/
├─ vendor/TakeCEngine/  # git submodule
├─ Source/              # ゲーム固有コード
├─ Assets/              # ゲーム固有アセット
└─ premake.lua
```

ゲーム側のPremakeからエンジンの共通定義を読み込みます。

```lua
local engineRepository = "vendor/TakeCEngine"
dofile(path.join(engineRepository, "premake/TakeCEngineProject.lua"))

DefineTakeCEngineProject {
    repositoryRoot = engineRepository,
    projectLocation = "build/projects/TakeCEngine"
}

project "NewGame"
    -- ゲーム側のfiles、targetdir、objdirなどを設定
    ConfigureTakeCEngineConsumer {
        repositoryRoot = engineRepository
    }
```

`ConfigureTakeCEngineConsumer`はincludeパス、静的ライブラリ参照、DirectML、ONNX Runtime、
Assimpなどのリンク設定と実行時DLLのコピーを設定します。

### リソースルート

ゲーム開始時に、ゲーム資産とエンジン資産のルートを明示します。

```cpp
TakeC::ResourceRootConfig resources{
    .gameRoot = "Assets",
    .engineRoot = "vendor/TakeCEngine/project/EngineContent",
};
game->Run(L"NewGame", resources);
```

モデル、画像、音声、JSON、ONNXモデルはゲームルートから、シェーダー、フォント、エンジン
既定画像はエンジンルートから解決されます。互換性維持のため、未指定時の既定値は両方とも
`Resources`です。

### 履歴を保って2リポジトリへ分割する手順

リモートリポジトリを用意した後、現在の作業をコミットし、必ず新しいclone上で
`git filter-repo`を実行します。元の作業ディレクトリでは実行しません。

1. エンジン用cloneでは`project/engine`、`project/EngineContent`、エンジンの外部依存、
   `premake/TakeCEngineProject.lua`、`premake/engine.lua`を残してエンジン用リモートへpushする。
2. ゲーム用cloneでは`project/application`、`project/Assets`、エントリーポイントとゲーム設定を
   残してゲーム用リモートへpushする。
3. ゲームリポジトリへエンジンを追加する。

```bat
git submodule add <TakeCEngineのURL> vendor/TakeCEngine
git submodule update --init --recursive
```

4. ゲーム側Premakeを上記の共通定義へ切り替え、Debug、Develop、ReleaseをCIで確認する。

実際の抽出対象とpush先を確定するには、エンジン用とゲーム用のリモートURLが必要です。
