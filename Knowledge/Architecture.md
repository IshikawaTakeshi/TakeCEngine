# TakeCEngineの構成

- GameObjectとComponentの実行時所有・更新は`project/engine/GameObject/`が担当する。
- GameObject中心のレベル文書・入出力・実体生成は`project/engine/Level/`が担当する。Editor UIの設計と進捗は[統合レベルエディタ](Features/LevelEditor.md)を参照。
- 汎用レベル編集UIは`project/engine/Editor/Level/`が担当し、LevelDocumentの変更・履歴はImGui非依存の`LevelEditorSession`へ委譲する。
