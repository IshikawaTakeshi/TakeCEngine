# AICommandBuffer

AIの判断と行動の実行を分離する、GameObject個体別の要求Bufferです。
AIControllerComponentが所有し、AIContext.commandsからStart/Tickへ渡します。
BehaviorTreeContextはAIContextの別名なので既存のActionにも同じBufferが届きます。

## 更新の流れ

```text
GameObjectManager::Update(deltaTime)
  Brain::Start/Tick → context.commandsへ要求を追加
GameObjectManager::LateUpdate(deltaTime)
  Controllerが要求を一度だけ消費
  → AICommandHandlerComponent::HandleCommand
  → AICommandHandlerComponent::UpdateExecution（毎フレーム）
```

毎フレーム、Updateの後にLateUpdateを呼んでください。FixedUpdateでは要求を消費しません。
ControllerのTick間隔が長くても、実行ComponentはLateUpdateごとに通常のフレーム時間で更新されます。
同じフレームのLateUpdateを重複して呼ぶと攻撃要求は再実行されませんが、
継続移動は呼び出した回数分だけ進むため、LateUpdateは1フレームに1回にします。
Brainが完了して判断を止めても、受け付け済みの移動などは到着・取消まで継続します。

## 要求と競合ルール

| API | 内容 |
| --- | --- |
| RequestMoveTo(worldPosition, stoppingDistance = 0.01f) | ワールド座標の移動先。移動速度・物理は実行側で決定 |
| RequestStopMovement() | 移動を停止 |
| RequestAttack(targetId, actionId = "Attack") | 対象IDとゲーム側の行動IDを渡す単発要求 |
| GetCommands() | 未消費要求の読み取り専用span。表示・診断用 |
| ConsumeAll() | 現在の要求を取り出してBufferから除去。通常はControllerが行う |
| Clear() | 未消費要求だけを破棄。既に開始した行動は停止しない |

同じBuffer内の移動/停止は最後の要求だけを残します。攻撃要求は追加順で残します。
置換した移動要求は、今回要求した位置に追加されます。ゲーム固有の優先順位付けは実装していません。
既定容量は64件です。満杯で追加できなければRequestはfalseを返し、既存の要求は保持します。
満杯でも既存の移動/停止要求の置換は可能です。攻撃だけで満杯なら移動・停止もfalseになります。
座標・停止距離の非有限値、負の停止距離、無効対象ID、空行動IDはinvalid_argumentです。
実在する対象か、攻撃可能かはBufferでは判定せず、実行側でIDを解決して確認します。

Successは「要求を受け付けた」という意味であり、移動到着・攻撃完了ではありません。
完了を待つActionは実行Componentの状態やゲーム側の完了通知を別途確認してください。
受付と実行の間にはLateUpdateがあるので、要求直後の状態を完了判定に使わないでください。

## BehaviorTreeからの使用

```cpp
#include "engine/GameObject/BehaviorTreeComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/GameObjectManager.h"

TakeC::GameObjectManager objects;
auto& enemy = objects.CreateObject("Enemy");
auto& movement = enemy.AddComponent<TakeC::AIMovementComponent>();
movement.SetSpeed(2.0f);
auto& behavior = enemy.AddComponent<TakeC::BehaviorTreeComponent>();
behavior.SetTickInterval(0.1f);
behavior.SetRepeat(false);
behavior.SetTree(std::make_unique<ActionNode>("MoveTo"));
behavior.BindAction("MoveTo", [](const TakeC::AIContext& context) {
    return context.commands.RequestMoveTo({10.0f, 0.0f, 0.0f})
        ? BehaviorStatus::Success : BehaviorStatus::Failure;
});

// 毎フレーム。判断が完了しても移動は継続する。
objects.Update(deltaTime);
objects.LateUpdate(deltaTime);
```

自作IAgentBrainもTick内で同じRequest APIを使います。
直接AIContextを構築するコードは `{owner, blackboard, commands, deltaTime}` としてください。
旧Execute(Blackboard&)だけのノードにはBufferが渡らないため、
命令を発行する独自ノードはTick(const AIContext&)をoverrideします。
JSON資産・ACTION.targetState・Action登録の形式は変更していません。

## 実行Componentを自作する

AICommandHandlerComponentを継承し、担当する要求だけHandleCommandで処理します。
trueを返した最初の有効Handlerがその要求を担当し、後続Handlerへは渡しません。
HandleCommandの呼び出し順はComponentの追加順です。
trueは担当/消費を意味し、攻撃が成功したことを意味しません。
ゲーム側で拒否した攻撃も消費して、翌フレームに再攻撃しないようにします。

以下はゲーム側の攻撃処理をコールバックで接続する例です。

```cpp
#include "engine/GameObject/AICommandHandlerComponent.h"
#include "engine/GameObject/GameObjectManager.h"
#include <functional>

class CombatComponent final : public TakeC::AICommandHandlerComponent {
public:
    using Attack = std::function<void(TakeC::GameObject&, const std::string&)>;
    CombatComponent(TakeC::GameObjectManager& objects, Attack attack)
        : objects_(objects), attack_(std::move(attack)) {}
    bool HandleCommand(const TakeC::AICommand& command) override {
        const auto* attack = std::get_if<TakeC::AIAttackCommand>(&command);
        if (!attack) { return false; }
        auto* target = objects_.FindObject(attack->targetId);
        if (target && target->IsActive() && !target->IsDestroyRequested() && attack_) {
            attack_(*target, attack->actionId); // 射程・クールダウン等はゲーム側で検証する
        }
        return true;
    }
private:
    TakeC::GameObjectManager& objects_;
    Attack attack_;
};
```

継続行動はUpdateExecution(context)へ実装し、CancelExecution()で状態を解除します。
同じ行動を自分のUpdate/LateUpdateでも実行すると二重更新になるため避けてください。
取り付け時の初期化にはOnCommandAttach、取り外し時にはOnCommandDetachを使えます。
HandlerはOnAttach後から要求を受け取り得るため、受付に必要な初期化をStartだけに依存させないでください。
対象のGameObjectを生ポインタで要求に保持せず、実行時にGameObjectIdから検索します。
Componentの追加順にかかわらず、Brainの通常Updateが済んでから要求が実行されます。

既定ではownerの最初のAIControllerが各Handlerを更新します。
同じGameObjectへ複数Controllerを載せる場合は、取り付け後に
`handler.SetCommandSource(&controller)`で同じownerのControllerを選択できます。
各Handlerは選択したControllerからだけ更新されるため、移動を二重更新しません。
未対応または無効Handlerへの要求は消費して破棄し、
Controller.GetLastUnhandledCommandCount()へ最後のLateUpdateの件数を記録します。

AIMovementComponentは経路探索・衝突・重力を伴わない直線移動の実装です。
親付きTransformもワールド座標で扱いますが、親の行列は有限かつ逆行列を持つ必要があります。
物理やA*を導入するときは独自Handlerへ置き換えてください。

## 一時停止・取消・例外

- Controller.Pause()は判断だけを止め、既に受け付けた行動は継続します。
- Controller/HandlerのSetEnabled(false)、ownerのSetActive(false)は実行も凍結します。
- ControllerのStop/Reset/Restart/Brain交換、BehaviorTreeComponentのツリー交換・Action再登録は
  未消費要求を破棄し、該当HandlerのCancelExecutionを呼びます。
- 自動Repeat時のツリーResetは、受け付け済みの行動を取り消しません。
- Handler無効中に消費した要求は破棄します。再度有効にしてもその単発要求は復活しません。
- Handler内で新しく追加した要求は、消費スナップショットとは別の次回LateUpdateへ送ります。
- Start/Tick/実行処理で例外が発生した場合、未消費要求と継続行動を破棄して例外を伝播します。
- Handler内でowner.Destroy()したら残りの要求を実行せず、既存の遅延破棄へ委ねます。

実行中もSetBrain/SetTree/Stop等の構造・ライフサイクル変更はlogic_errorです。
破棄要求はowner.Destroy()を使えます。CancelExecutionは例外を投げないでください。
要求は常時蓄積し、消費・取消時に消えます。各Tickの冒頭では消去しないため、
Startの要求やLateUpdate前の要求が次の判断で失われることはありません。
BufferとHandlerはゲームループの同じスレッドで使い、参照をフレームをまたいで保持しないでください。

## 検証

`project\tests\RunBehaviorTreeComponentTests.bat`で既存BTテストと
`AICommandBufferTests.cpp`をまとめてビルド・実行します。
容量・競合・入力検証・個体分離、低頻度Brainと毎フレーム移動、
JSON Actionからの攻撃、単発消費と消失対象、停止/交換、親Transform、
複数Controller、スナップショット、例外・遅延破棄を検証します。
