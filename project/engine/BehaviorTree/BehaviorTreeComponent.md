# GameObjectからBehaviorTreeを実行する

`TakeC::BehaviorTreeComponent`は共通の`AIControllerComponent`を継承し、
`IAgentBrain`を実装した`BehaviorTreeBrain`を搭載する便利Componentです。
Controllerが個体別Blackboard・AICommandBufferと更新を管理し、
Brainがツリーとアクション登録を所有します。
GameObjectManagerの`Update(deltaTime)`、またはGameObjectの`Update(deltaTime)`を呼ぶと
毎フレーム1回ツリーを実行します。FixedUpdateとLateUpdateではBrainは実行しません。
行動要求はLateUpdateで消費するため、毎フレームUpdateの後にLateUpdateも呼びます。

## 共通AIインターフェース

`AIContext`は`owner`、`blackboard`、`commands`、`deltaTime`を渡します。
`BehaviorTreeContext`は互換性維持のための`AIContext`の別名です。
`IAgentBrain`の`Start(AIContext&)`、`Tick(AIContext&)`、`Reset(AIContext&)`で
BehaviorTreeと自作AIを同じControllerから扱えます。
`IsFinished()`がtrueならTick後にControllerが停止します（既定false）。

```cpp
#include "engine/GameObject/AIControllerComponent.h"
#include "engine/BehaviorTree/BehaviorTreeBrain.h"

auto& controller = enemy.AddComponent<TakeC::AIControllerComponent>();
auto brain = std::make_unique<TakeC::BehaviorTreeBrain>();
brain->SetTree(factory.BuildBehaviorTree(asset));
brain->BindAction("Game.Move", moveCallback); // AIContextを受け取る
controller.SetBrain(std::move(brain));
controller.SetTickInterval(0.1f); // 省略すると毎フレーム

// 自作AI（または今後実装するFSM）へ差し替え。個体別Blackboardは維持される。
controller.SetBrain(std::make_unique<MyBrain>());
```

MyBrainは`IAgentBrain`を継承してTickを実装します。
汎用FSMとして[StateMachineBrain](../AI/StateMachineBrain.md)も利用できます。
便利Componentである`BehaviorTreeComponent`のSetBrainはBehaviorTreeBrainのみを受け付けます。
AI方式の交換には上記の汎用`AIControllerComponent`を利用してください。

BrainのStartは、取り付け後の最初の有効なUpdateで1回だけ呼ばれます。
Pause/Resume/RestartではStartをやり直しません。Brain交換後は新BrainをStartします。
ResetはStop、Restart、Brain交換、Detachで呼ばれ、Start前にも呼ばれる場合があります。
GameObjectへ未取り付けのControllerではownerがないためStart/Resetは呼びません。
ComponentのHasStartedはComponent側の状態であり、BrainのStart状態ではありません。

Tick間隔が0なら毎フレーム、正数なら有効なUpdateの時間を蓄積して実行します。
超過分も含めた全時間をdeltaTimeに渡し、1フレームに複数回の追いつき実行はしません。
Pause/Disable/Inactive中の時間は加算せず、途中までの蓄積は保持します。
Reset、交換、ツリー変更、アクション再登録、間隔変更では蓄積時間を破棄します。
間隔とdeltaTimeは有限かつ0以上でなければなりません。

AICommandBufferと実行Componentの接続を実装しています。
Actionはcommandsへ要求を書き、ControllerのLateUpdateで実行側へ渡します。
使用方法・命令の寿命・自作実行Componentは[AICommandBuffer](../AI/AICommandBuffer.md)を参照してください。
World参照とPerception/Decision等の更新順序は共通AIContextとComponentPhaseで利用できます。
NavigationAgentによる経路探索は今後の実装です。

## コードでツリーを組み立てる

```cpp
#include "engine/GameObject/BehaviorTreeComponent.h"
#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/BehaviorTree/SequenceNode.h"
#include "engine/BehaviorTree/WaitNode.h"

TakeC::GameObjectManager objects;
auto& enemy = objects.CreateObject("Enemy");
auto& movement = enemy.AddComponent<TakeC::AIMovementComponent>();
movement.SetSpeed(2.0f);
auto& behavior = enemy.AddComponent<TakeC::BehaviorTreeComponent>();

behavior.BindAction("Game.Move", [](const TakeC::BehaviorTreeContext& context) {
    auto target = context.owner.GetTransform().GetWorldPosition();
    target.x += 2.0f;
    return context.commands.RequestMoveTo(target)
        ? BehaviorStatus::Success : BehaviorStatus::Failure;
});

auto sequence = std::make_unique<SequenceNode>();
sequence->AddChild(std::make_unique<WaitNode>(1.0f));
sequence->AddChild(std::make_unique<ActionNode>("Game.Move"));
behavior.SetTree(std::move(sequence));

// 毎フレーム。1秒ごとに移動先を更新し、毎LateUpdateで移動する。
objects.Update(deltaTime);
objects.LateUpdate(deltaTime);
```

処理が継続中ならアクションは`Running`、完了なら`Success`、
実行できなければ`Failure`を返します。上のSuccessは要求の受付成功であり、到着完了ではありません。
命令が満杯などで受付失敗ならFailureを返します。自作Componentは
`context.owner.GetComponent<MyComponent>()`で取得できます。
同じアクションIDを異なる個体へ登録し、それぞれ別の処理を設定できます。

## JSON資産を利用する

既存の`BehaviorTreeFactory`が返すツリーを`SetTree`へ渡します。
資産データは共有できますが、実行時ツリーは個体ごとにFactoryで生成してください。

```cpp
TakeC::BehaviorTreeFactory factory;
auto root = factory.LoadBehaviorTreeAsset("Enemy.json");
if (root) {
    behavior.SetTree(std::move(root));
    behavior.BindAction("Game.Move", moveCallback);
}
```

ACTIONノードのアクションIDは`properties.actionId`に保存します。既存JSONの`ACTION.targetState`も読み込めます。
登録はSetTreeの前後どちらでもできます。
組み込みACTIONに登録がなく、ノード自身にも実行コールバックがなければFailureになります。
独自ノードは既存のBehaviorNodeRegistryへ登録できます。

## 実行と停止

| API | 動作 |
| --- | --- |
| SetTree(root) | 旧ツリーをResetして交換し、実行を開始 |
| SetTree(nullptr) | 旧ツリーをResetして解除 |
| Pause / Resume | 途中状態を保持して停止・再開 |
| Stop | ツリーをResetして停止、状態はInvalid |
| Restart | ツリーをResetして先頭から再開 |
| Reset | ツリーをReset、再生・一時停止設定は保持 |
| SetRepeat(false) | SuccessまたはFailureで停止する1回実行 |
| GetStatus() | 最後の実行結果。次の実行まで保持 |
| GetTree() | Editorへ渡す実行時root |

既定は繰り返し実行です。SuccessまたはFailureの次フレームにResetして再実行します。
AbortedまたはInvalidは停止し、ResumeまたはRestartで再実行できます。
Blackboardの値は停止・再実行・ツリー交換でも保持します。
値も消す場合は`GetBlackboard().Clear()`を明示的に呼びます。

`SetEnabled(false)`と所有GameObjectの`SetActive(false)`は状態を保持した一時停止です。
停止に伴うアクションの終了処理が必要なら先にStopを呼びます。
Pauseは判断だけを止めます。受け付け済みの行動も止めるにはStopを使います。
Stop/Reset/Restart/Brain交換/ツリー交換/Action再登録ではBufferを消去し、実行側の行動も取り消します。
GameObjectの破棄時にもResetが呼ばれ、登録したresetコールバックが実行されます。
resetは通常完了時にも呼ばれます。繰り返し呼べる、例外を投げない処理にしてください。
キャプチャするオブジェクトはComponentの破棄まで有効である必要があります。

## 自作ノードと互換性

新しいノードは`Tick(TakeC::AIContext&)`のみを実装できます。
既存の`Execute(Blackboard&)`のみ、または旧`Tick(const TakeC::BehaviorTreeContext&)`を実装したノードも利用できます。
命令は`context.commands`へ発行します。Contextを直接生成する場合は
`{owner, blackboard, commands, deltaTime}`としてください。
独自Compositeでは子ノードにも`Tick(context)`を呼んでContextを伝えます。
Context参照をフレームをまたいで保存してはいけません。

組み込みSequence、Selector、PlannerSelector、WeightSelectorはContextを子へ伝えます。
WaitとWaitBlackboardTimeはComponentから渡されたdeltaTimeを使います。
旧Executeを直接呼んだ場合は従来どおりTakeCFrameWorkの時計を使います。

Start、Tick、Reset、命令の実行コールバック中にSetBrain、SetTickInterval、
SetTree、BindAction、Stop、Restart、Resetを呼ぶと
`std::logic_error`になります。構造変更はGameObjectの更新外で行ってください。
PauseとGameObject::Destroyによる遅延破棄要求はコールバック内でも利用できます。
SensorComponentはPerception、AIControllerはDecisionで更新され、追加順に関係なく感知が先になります。

## 検証

```bat
project\tests\RunBehaviorTreeComponentTests.bat
project\tests\RunGameObjectTests.bat
```

BehaviorTreeComponentのテストは描画・ウィンドウを初期化せずに実行します。
JSON資産からの生成、4種類のCompositeのContext伝達と中断伝達、待機時間、
個体別Blackboard、旧ノード互換、一時停止・停止・再実行・遅延破棄を検証します。
共通Controller単独でのBehaviorTree実行と自作Brainへの交換、Start/Resetのライフサイクル、
BrainとBlackboardの個体分離、更新間隔の時間蓄積、不正な間隔、再入防止も検証します。
同じ実行スクリプトでAICommandBufferTests.cppも実行します。
StateMachineBrainTests.cppと巡回・追跡・攻撃のサンプルも同じスクリプトで検証します。

## 実装案の進捗

| 順序 | 項目 | 状況 |
| --- | --- | --- |
| 1 | ComponentUpdateContextと更新フェーズ | 共通ContextとUpdateでの判断/LateUpdateでの実行を実装。詳細なPerception/Navigation等の分離は未実装 |
| 2 | AIContext・IAgentBrain・AIControllerComponent | 基本契約・ライフサイクル・個体別Blackboard/Commands・更新間隔は実装済み。ContextのWorldは未実装 |
| 3 | AICommandBufferと実行Component | 移動/停止/攻撃要求、共通Handler、直線移動Componentを実装。ゲーム固有の攻撃・物理処理はゲーム側で接続 |
| 4 | StateMachineBrain | 実装済み。個体別State、Enter/Tick/Exit、条件/明示要求の優先順位、Reset/Restartと命令発行、サンプル・テストに対応 |
| 5 | BehaviorTreeのAIContext対応 | 実装済み。BehaviorTreeBrainへ分離し、既存Component API・JSON・旧Executeとの互換を維持 |
| 6 | NavigationGraph・A*・NavigationAgentComponent | 保留（依頼により今回の対象外） |
| 7 | Prefab/JSONによるComponentとAI構成の生成 | BT資産からツリー生成は対応済み。Component/Brain構成の自動生成は未実装 |
| 8 | Face_GameのEnemy 1体の移行 | 未実施 |
| 9 | デバッグ表示と単体テスト | Controller/Brainのテストを追加。既存BT Editorへroot/BBは渡せるが、Component専用Inspectorは未実装 |
| 10 | 残りのゲームオブジェクトを段階移行 | 未実施 |
