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

## エンジンとしての利用

Premakeで次の2プロジェクトを生成します。

- `TakeCEngine`: `project/engine`をビルドする静的ライブラリ
- `DirectXGame`: 現在の`project/application`を使うサンプルゲーム

これにより、現在のリポジトリをエンジン兼サンプルとして残し、新しいゲーム側から
`TakeCEngine.vcxproj`をサブプロジェクトとして参照できます。

新しいゲームリポジトリでは、例えば次の構成を想定します。

```text
NewGame/
├─ vendor/TakeCEngine/       # git submodule
├─ Source/                   # 新ゲーム固有コード
├─ Resources/                # 新ゲーム固有アセット
└─ NewGame.sln
```

```bat
git submodule add https://github.com/IshikawaTakeshi/TakeCEngine vendor/TakeCEngine
vendor\TakeCEngine\premake\GenerateProject.bat v143
```

生成された`vendor/TakeCEngine/project/TakeCEngine.vcxproj`を新ゲームのソリューションへ追加し、
ゲームプロジェクトからプロジェクト参照を設定します。ローカル環境でv145を使う場合は、
バッチの引数を`v145`にします。

### プロジェクト生成とビルド

必要なNuGetパッケージは`project/packages.config`に定義されています。

```bat
nuget restore project\packages.config -PackagesDirectory project\packages
premake\GenerateProject.bat v143
msbuild project\DirectXGame.sln /p:Platform=x64 /p:Configuration=Debug /p:PlatformToolset=v143 /m
```

構成は`Debug`、`Develop`、`Release`の3種類です。GitHub Actionsも、NuGet復元、Premake生成、
MSBuildの順で同じ設定を再現します。

### リソースルート

リソースパスは`Resources/...`を各機能へ直書きせず、`TakeC::ResourcePath`で解決します。
ゲーム開始時にゲーム用とエンジン用のルートをそれぞれ指定できます。

```cpp
TakeC::ResourceRootConfig resources{
    .gameRoot = "Resources",
    .engineRoot = "vendor/TakeCEngine/project/Resources",
};
game->Run(L"NewGame", resources);
```

既存ゲームとの互換性のため、未指定時は両方とも`Resources`です。新ゲーム側のモデル、画像、
音声、JSON、ONNXモデルはゲームルートから、シェーダー、フォント、エンジン既定画像は
エンジンルートから解決されます。

現時点では`TakeCFrameWork`が既存のシーン抽象クラスを参照しているため、Premake上では
一部の`application/Scene`ファイルも移行用としてエンジンライブラリへ含めています。
完全分離の次段階では、これらを`engine/Scene`へ移動してアプリケーションへの逆依存を解消します。
