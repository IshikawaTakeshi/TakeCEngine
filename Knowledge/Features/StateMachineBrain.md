# StateMachineBrain（汎用FSM）

IAgentBrainを実装したFSMです。GameObjectのAIControllerComponentへSetBrainで搭載します。
Controller、AIContext、個体別Blackboard/AICommandBuffer、実行Handlerは
BehaviorTreeBrainと共通で、FSM専用Componentを追加する必要はありません。
BehaviorTreeComponentはBT専用なので、FSMには汎用AIControllerComponentを使います。

## ステートの実装

IAgentStateを継承し、Enter/Tick/Exitを実装します。

- Enterは入場時に1回。タイマー初期化、移動先指定、単発要求などを行います。
- TickはBrainの判断更新ごとに1回。deltaTimeはControllerが蓄積した判断間隔です。
- Exitは離脱時に1回。Reset/Stop/交換/Detach時にも呼ばれます。例外を投げないでください。

ステートのインスタンスはBrainごとにunique_ptrで所有します。
タイマー等の個体別状態をstaticや共有インスタンスへ置かないでください。
Context参照は呼び出し中だけ有効です。実際の移動・攻撃はcommandsへ要求し、
LateUpdateの実行Handlerへ委ねます。

```cpp
class IdleState final : public TakeC::IAgentState {
public:
    void Enter(TakeC::AIContext&) override { elapsed_ = 0.0f; }
    void Tick(TakeC::AIContext& context) override {
        elapsed_ += context.deltaTime;
        context.blackboard.Set("idleTime", elapsed_);
    }
private:
    float elapsed_ = 0.0f;
};

auto brain = std::make_unique<TakeC::StateMachineBrain>();
brain->AddState("Idle", std::make_unique<IdleState>());
brain->SetInitialState("Idle");
controller.SetBrain(std::move(brain));
```

## APIと遷移規則

| API | 動作 |
| --- | --- |
| AddState(id, state) | 空でない一意IDと個体専用ステートを登録 |
| SetInitialState(id) | 登録済み初期IDを設定。現在状態は変えず、次の開始/Reset後に適用 |
| AddTransition(from, to, condition, priority = 0) | 登録済みID間の条件遷移を追加 |
| RequestTransition(to, priority = 0) | 次回Tickの明示的な遷移を予約 |
| GetCurrentStateId() | 現在ID。未開始/Reset後は空。viewは遷移・Reset・破棄まで有効 |
| GetInitialStateId() | 初期ID |
| IsActive() | 実行中のステートがあるか |
| GetStateCount() / GetTransitionCount() | 登録された定義数 |

1Tickの流れは、未開始なら初期Enter、条件/予約の判定、
必要なら旧Exit→新Enter、現在ステートのTickです。
旧ステートをTickしてから遷移することはなく、1Tickで最大1回だけ遷移します。
新ステートの遷移条件を同じTickで続けて判定しません。

優先順位が大きい候補を選びます。同順位の条件は登録順、
条件と明示要求が同順位なら明示要求を優先します。
負の優先順位も使えます。明示要求を複数回発行した場合は、
高優先順位の要求を保持し、同順位なら最後の要求に置き換えます。
条件に負けた明示要求は、その判断で消費して翌回へ持ち越しません。
すでに選んだ候補より優先順位が低い/同じ条件は評価を省略します。

現在状態への明示要求は再Enterせずに消費します。他の条件は通常どおり評価します。
同一IDへの条件遷移は、再入・条件の遮蔽を避けるため登録時に拒否します。
未登録ID、空ID、nullステート、重複ステート、空条件はinvalid_argumentです。
初期状態を未設定のままStart/Tickするとlogic_errorになります。
Start/TickのdeltaTimeは有限かつ0以上でなければなりません。

Enter/Tick/Exit内のRequestTransitionは即時遷移せず、次のTickへ予約します。
StartのEnterで予約した場合は、Startの後の最初のTickで判定します。
Tick自身が初期Enterした場合は、そのEnterでの要求は次のTickまで延期します。
条件コールバックは判断だけにし、Blackboard・命令・GameObjectを変更しないでください。
条件からRequestTransitionを呼ぶとlogic_errorです。
条件を含む実行コールバック内での定義追加/初期ID変更/再帰的Start/Tick/Resetも拒否します。

## ライフサイクル

Startは初期状態に入ります。すでに開始済みなら何もしません。
ControllerのStart時点ではdeltaTimeは0で、Tick間隔に達する前にもEnterの命令は実行され得ます。
Pause/Resumeは現在状態を保持し、Enterをやり直しません。

Resetは現在状態をExitし、現在IDと予約要求を解除します。
登録済みステート、遷移、初期ID、Blackboardは維持します。
Reset中のExitが発行した予約も破棄します。未開始/繰り返しResetはExitを重複させません。
ControllerはRestart時にBrain.Startを再呼び出さないため、
Reset後は次の判断Tickで初期Enterを行います（Tick間隔待ちも通常どおり適用）。
FSMは既定で継続動作し、IsFinishedはfalseです。完了時の停止はController.Stop等で行います。

通常の状態遷移ではCommandBuffer全体や実行中の行動を自動消去しません。
必要な停止はExit/EnterからRequestStopMovementで明示します。
要求の受付成功は行動完了ではありません。到着や攻撃終了は実行Component/完了通知を別途確認します。
ControllerのStop/Reset/Restart/Brain交換/Detachは、既存の取消処理を利用します。

Enter/Tick/条件のコールバックで例外が発生した場合は、現在状態をExitして実行状態と予約を解除し、
例外を呼び出し元へ伝えます。Enter途中の失敗にもExitが呼ばれるため、
Exitは部分初期化状態にも対応させてください。
Exitが契約に反して例外を投げても同じステートのExitは重複させませんが、
終了処理の失敗は正常動作として扱いません。
不正なdeltaTimeはコールバックを呼ぶ前に拒否し、現在状態と予約を維持します。
Controller経由なら命令/継続行動も取り消されます。
Brainを直接使う場合、Bufferの消去・実行Handlerの取消と最終Resetは呼び出し側が担当します。

## 巡回・追跡・攻撃の使用例

実際にコンパイルするゲーム側サンプルは
[StateMachineBrainExample.h](../../examples/StateMachineBrainExample.h)です。
エンジン本体からサンプルへの依存はありません。

```cpp
#include "examples/StateMachineBrainExample.h"
#include "engine/GameObject/AIControllerComponent.h"
#include "engine/GameObject/AIMovementComponent.h"

TakeC::GameObjectManager objects;
auto& target = objects.CreateObject("Target");
auto& enemy = objects.CreateObject("Enemy");
auto& movement = enemy.AddComponent<TakeC::AIMovementComponent>();
movement.SetSpeed(2.0f);
auto& controller = enemy.AddComponent<TakeC::AIControllerComponent>();
controller.SetBrain(TakeC::Examples::BuildEnemyBrain(
    objects, target.GetId(), {-5.0f, 0.0f, 0.0f}));

// 視認と射程はゲーム側で計算して、毎フレーム判断より前に更新する。
controller.GetBlackboard().Set("enemyVisible", true);
controller.GetBlackboard().Set("inAttackRange", false);
objects.Update(deltaTime);
objects.LateUpdate(deltaTime);
```

Patrolは巡回先へ移動、Chaseは対象IDから現在位置を再取得して追跡、
Attackは入場時に停止と単発攻撃を要求します。
敵を見失う条件を射程条件より高優先順位にしています。
1Tickに1遷移なので、巡回中に視認と射程内が同時に成立しても
Patrol→Chase→Attackを2回の判断に分けます。
攻撃実行はゲーム側のAICommandHandlerComponentを別途取り付けてください。
サンプルのAttackは1入場1攻撃です。連続攻撃・クールダウン・ダメージはゲーム側へ実装します。
GameObjectManagerはステートより長く有効である必要があります。
知覚の計算、Face_GameのEnemyへの移行、階層FSM、並列状態、FSMのJSON/Editorは対象外です。

## 検証

`project\tests\RunBehaviorTreeComponentTests.bat`で既存BT/CommandBufferテストと
[StateMachineBrainTests.cpp](../../tests/StateMachineBrainTests.cpp)をまとめて実行します。
遷移順序・回数、条件/要求の優先順位、Enter/Tick/Exitからの予約、
Reset/Restart/交換/Detach、入力・再入・例外、個体分離、
実サンプルの巡回→追跡→攻撃とLateUpdateでの命令実行を検証します。
