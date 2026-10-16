# Prefab生成（version 1）

単一GameObjectをJSONから生成する。既存LevelObjectの読み込み形式は変更しない。
PrefabFactory.hは描画に依存しない。描画・衝突はPrefabRendering.hの
RegisterPrefabRendering(factory, object3dCommon)で登録する。

## 使用

- Instantiate(world, json): 成功したGameObjectだけをManagerへ登録する。
- Load(world, path): JSONファイルを読み込んでInstantiateする。
- Build(json): 未登録のunique_ptrを返す。world.AdoptObjectで登録できる。
- project/examples/PrefabExample.hにBTアセット読み込みを含む使用例がある。
- project/examples/Prefabsをアセットルートとし、PatrolEnemy.jsonを指定する。
  EnemyActionsにはBindAction("Patrol", ...)を行う関数を渡す。
  サンプルはモデル未指定なので描画されない。必要に応じて下記Componentを追加する。

## フィールド

必須: version=1、components配列。任意: name、active、transform。
transformはposition/rotation/scaleを各[x,y,z]で指定する。rotationはEuler角（ラジアン）。
TransformはGameObject標準のものを設定し、components配列へは追加しない。
未知のフィールド、未知のComponent、同じtypeの重複、複数AIControllerはエラー。

Component:

- AIMovement: speed（0以上）。直線移動。物理・経路探索は含まない。
- BehaviorTree: tree、任意actionSet/repeat/tickInterval/blackboard。
  TreeLoaderは呼び出すたびに新しいツリーを返すこと。パス解決はLoader側で行う。
- AIController: brain、任意settings/tickInterval/blackboard。
  RegisterBrain(name, builder)でFSMまたは自作IAgentBrainの生成関数を登録する。
  FSMの状態・遷移の汎用JSON記法は今回の対象外。
- ModelRenderer（追加登録）: model。ModelManagerでロードしてからRendererを初期化。
- Collision（追加登録）: shape="Box"とhalfExtents、またはshape="Sphere"とradius。
  任意center/layer/mask/trigger。寸法とcenterはローカル座標で、スケールを事前に掛けない。
  layer/maskはuint32ビットマスク。デフォルトは0/0xffffffff。

Blackboard初期値はbool/string/int32/floatをサポートする。
整数記法はint32、小数記法はfloat。Vector/Object IDは初回仕様では未対応。
ActionSetは個体ごとに呼ばれる。状態を共有する可変キャプチャや、生成関数の
一時変数への参照をコールバックへ保存しないこと。

## 拡張・寿命

RegisterComponent(type, builder)で独自Componentを登録可能。builder内で
GameObject.AddComponentを呼び、設定を検証する。builderは未登録かつ非アクティブな
GameObjectを受け取るので、OnAttachでWorld/EngineServicesの存在を前提にしない。
サービス利用は登録後のInitialize/Updateまで遅延する。
登録済みコールバックが参照するサービスは生成処理より長く生存させる。

例外時は途中のGameObjectを破棄し、CollisionComponentも登録解除する。
Manager内の既存オブジェクトは変更しない。ただしロード済みモデルの共有キャッシュや、
独自builderが行った外部副作用は巻き戻さない。builderはRAIIで資源を管理すること。
描画登録は有効な描画環境でのみ使う。生成後のDraw/DrawShadowはScene側で行う。
親子階層、継承、保存、ホットリロード、専用Editorは対象外。

## 検証

RunPrefabTests.batでPrefabTests.hを実行する。
正常生成、個体分離、Managerへの所有権移譲、独自Brainの実行、
未知型・重複・不正値・途中失敗時にManagerへ残らないことを検証する。
描画・衝突の実機表示は別途ゲーム起動で確認する。
