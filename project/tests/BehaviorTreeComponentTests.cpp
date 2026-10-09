#include "engine/GameObject/BehaviorTreeComponent.h"
#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/BehaviorTree/ComboFactory.h"
#include "engine/BehaviorTree/PlannerSelectorNode.h"
#include "engine/BehaviorTree/SelectorNode.h"
#include "engine/BehaviorTree/SequenceNode.h"
#include "engine/BehaviorTree/WaitBlackboardTimeNode.h"
#include "engine/BehaviorTree/WaitNode.h"
#include "engine/BehaviorTree/WeightSelectorNode.h"
#include "engine/base/TakeCFrameWork.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

// 旧Execute用のグローバル時計。ComponentのdeltaTimeと異なる値で独立性を検証します。
float TakeC::TakeCFrameWork::deltaTime = 0.9f;

// テストではFactoryへ読み込んだAssetを渡し、フレームワークのSingletonは使いません。
TakeC::JsonLoader* TakeC::TakeCFrameWork::GetJsonLoader() { return nullptr; }

void RunAICommandBufferTests();

namespace {

using TakeC::BehaviorTreeComponent;
using TakeC::BehaviorTreeContext;
using TakeC::GameObject;

static_assert(std::is_base_of_v<TakeC::IAgentBrain, TakeC::BehaviorTreeBrain>);
static_assert(std::is_base_of_v<TakeC::AIControllerComponent, BehaviorTreeComponent>);
static_assert(std::is_same_v<TakeC::AIContext, BehaviorTreeContext>);

struct BrainProbe {
    int starts = 0;
    int ticks = 0;
    int resets = 0;
    float lastDeltaTime = 0.0f;
    GameObject* owner = nullptr;
    TakeC::Blackboard* blackboard = nullptr;
    TakeC::AICommandBuffer* commands = nullptr;
};

class CustomBrain final : public TakeC::IAgentBrain {
public:
    explicit CustomBrain(BrainProbe& probe, bool finishAfterTick = false)
        : probe_(probe), finishAfterTick_(finishAfterTick) {}
    void Start(TakeC::AIContext& context) override {
        ++probe_.starts;
        probe_.owner = &context.owner;
        probe_.blackboard = &context.blackboard;
        probe_.commands = &context.commands;
        assert(context.deltaTime == 0.0f);
    }
    void Tick(TakeC::AIContext& context) override {
        ++probe_.ticks;
        probe_.lastDeltaTime = context.deltaTime;
        context.blackboard.Set("customTicks", probe_.ticks);
        finished_ = finishAfterTick_;
    }
    void Reset(TakeC::AIContext& context) override {
        ++probe_.resets;
        assert(&context.owner == probe_.owner || probe_.starts == 0);
        assert(context.deltaTime == 0.0f);
        finished_ = false;
    }
    bool IsFinished() const override { return finished_; }
private:
    BrainProbe& probe_;
    bool finishAfterTick_ = false;
    bool finished_ = false;
};

void TestCommonControllerLifecycleAndBrainReplacement() {
    BrainProbe first;
    BrainProbe second;
    {
        GameObject object("CustomAI");
        auto& controller = object.AddComponent<TakeC::AIControllerComponent>();
        controller.SetBrain(std::make_unique<CustomBrain>(first));
        controller.GetBlackboard().Set("persistent", 42);
        object.FixedUpdate(10.0f);
        object.LateUpdate(10.0f);
        assert(first.starts == 0 && first.ticks == 0);
        object.Update(0.25f);
        object.Update(0.125f);
        assert(first.starts == 1 && first.ticks == 2);
        assert(first.owner == &object);
        assert(first.blackboard == &controller.GetBlackboard());
        assert(first.commands == &controller.GetCommandBuffer());
        assert(first.lastDeltaTime == 0.125f);
        controller.Pause();
        object.Update(10.0f);
        assert(first.ticks == 2);
        controller.Restart();
        assert(first.resets == 1);
        object.Update(0.125f);
        assert(first.starts == 1 && first.ticks == 3);
        controller.SetBrain(std::make_unique<CustomBrain>(second, true));
        assert(first.resets == 2 && second.starts == 0);
        assert(controller.GetBlackboard().GetOr<int>("persistent", 0) == 42);
        object.Update(0.25f);
        assert(second.starts == 1 && second.ticks == 1);
        assert(!controller.IsPlaying());
        object.Update(10.0f);
        assert(second.ticks == 1);
        controller.Restart();
        object.Update(0.25f);
        assert(second.starts == 1 && second.ticks == 2);
    }
    assert(second.resets == 2); // RestartとDetach。Detach時にも有効なownerを渡します。
}

void TestGenericControllerBehaviorTreeAndIsolation() {
    GameObject first;
    GameObject second;
    auto configure = [](GameObject& object) {
        auto& controller = object.AddComponent<TakeC::AIControllerComponent>();
        auto brain = std::make_unique<TakeC::BehaviorTreeBrain>();
        brain->SetTree(std::make_unique<ActionNode>("Count"));
        brain->BindAction("Count", [](const TakeC::AIContext& context) {
            const int ticks = context.blackboard.GetOr<int>("ticks", 0);
            context.blackboard.Set("ticks", ticks + 1);
            return BehaviorStatus::Running;
        });
        controller.SetBrain(std::move(brain));
    };
    configure(first);
    configure(second);
    first.Update(0.25f);
    second.Update(0.25f);
    first.Update(0.25f);
    auto& a = *first.GetComponent<TakeC::AIControllerComponent>();
    auto& b = *second.GetComponent<TakeC::AIControllerComponent>();
    assert(a.GetBlackboard().GetOr<int>("ticks", 0) == 2);
    assert(b.GetBlackboard().GetOr<int>("ticks", 0) == 1);
    assert(a.GetBrain() != b.GetBrain());
    a.SetBrain(nullptr);
    assert(a.GetBrain() == nullptr && !a.IsPlaying());
    first.Update(10.0f);
    assert(a.GetBlackboard().GetOr<int>("ticks", 0) == 2);
}

void TestControllerTickIntervalAndValidation() {
    BrainProbe probe;
    GameObject object;
    auto& controller = object.AddComponent<TakeC::AIControllerComponent>();
    controller.SetBrain(std::make_unique<CustomBrain>(probe));
    controller.SetTickInterval(0.5f);
    object.Update(0.125f);
    assert(probe.starts == 1 && probe.ticks == 0);
    controller.Pause();
    object.Update(10.0f);
    controller.Resume();
    controller.SetEnabled(false);
    object.Update(10.0f);
    controller.SetEnabled(true);
    object.SetActive(false);
    object.Update(10.0f);
    object.SetActive(true);
    object.Update(0.125f);
    object.Update(0.25f);
    assert(probe.ticks == 1 && probe.lastDeltaTime == 0.5f);
    object.Update(0.75f); // 間隔を超過しても時間を失わず、1更新につき1Tick。
    assert(probe.ticks == 2 && probe.lastDeltaTime == 0.75f);
    object.Update(0.25f);
    controller.Stop();
    controller.Resume();
    object.Update(0.25f);
    assert(probe.ticks == 2);
    object.Update(0.25f);
    assert(probe.ticks == 3 && probe.lastDeltaTime == 0.5f);
    for (float invalid : {-1.0f, std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()}) {
        bool rejected = false;
        try {
            controller.SetTickInterval(invalid);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected && controller.GetTickInterval() == 0.5f);
    }
    controller.SetTickInterval(0.0f);
    object.Update(0.125f);
    assert(probe.ticks == 4 && probe.lastDeltaTime == 0.125f);

    auto& tree = object.AddComponent<BehaviorTreeComponent>();
    tree.SetTree(std::make_unique<WaitNode>(1.0f));
    tree.SetRepeat(false);
    tree.SetTickInterval(0.5f);
    object.Update(0.25f);
    assert(tree.GetStatus() == BehaviorStatus::Invalid);
    object.Update(0.25f);
    assert(tree.GetStatus() == BehaviorStatus::Running);
    object.Update(0.25f);
    object.Update(0.25f);
    assert(tree.GetStatus() == BehaviorStatus::Success && !tree.IsPlaying());
    tree.SetTree(nullptr);
    tree.Restart();
    assert(!tree.IsPlaying());
}

void TestCommonControllerReentrantProtection() {
    class ReentrantBrain final : public TakeC::IAgentBrain {
    public:
        explicit ReentrantBrain(TakeC::AIControllerComponent& controller)
            : controller_(controller) {}
        void Start(TakeC::AIContext&) override { ExpectResetRejected(); }
        void Tick(TakeC::AIContext&) override {
            bool rejected = false;
            try {
                controller_.SetBrain(nullptr);
            } catch (const std::logic_error&) {
                rejected = true;
            }
            assert(rejected);
            controller_.Pause(); // 構造を変更しない一時停止は安全です。
        }
        void Reset(TakeC::AIContext&) override { ExpectResetRejected(); }
    private:
        void ExpectResetRejected() {
            bool rejected = false;
            try {
                controller_.Reset();
            } catch (const std::logic_error&) {
                rejected = true;
            }
            assert(rejected);
        }
        TakeC::AIControllerComponent& controller_;
    };
    GameObject object;
    auto& controller = object.AddComponent<TakeC::AIControllerComponent>();
    controller.SetBrain(std::make_unique<ReentrantBrain>(controller));
    object.Update(0.1f);
    assert(!controller.IsPlaying());
    controller.Stop();

    auto& tree = object.AddComponent<BehaviorTreeComponent>();
    TakeC::AIControllerComponent& base = tree;
    bool rejected = false;
    try {
        base.SetBrain(nullptr); // 便利Componentの型不変条件も基底経由で維持します。
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected && dynamic_cast<TakeC::BehaviorTreeBrain*>(tree.GetBrain()));

    tree.BindAction("ResetGuard", [](const TakeC::AIContext&) {
        return BehaviorStatus::Running;
    }, [&] {
        bool intervalRejected = false;
        try {
            tree.SetTickInterval(1.0f);
        } catch (const std::logic_error&) {
            intervalRejected = true;
        }
        assert(intervalRejected);
    });
    tree.SetTree(std::make_unique<ActionNode>("ResetGuard"));
    tree.BindAction("Other", [](const TakeC::AIContext&) {
        return BehaviorStatus::Success;
    }); // BindActionが呼ぶノードResetの再入も検証します。
    tree.SetTree(std::make_unique<WaitNode>(1.0f));
}

void TestInstanceIsolationAndLegacyNodes() {
    TakeC::GameObjectManager manager;
    auto& first = manager.CreateObject("First");
    auto& second = manager.CreateObject("Second");
    auto configure = [](GameObject& object) {
        object.AddComponent<TakeC::AIMovementComponent>();
        auto& component = object.AddComponent<BehaviorTreeComponent>();
        component.SetTree(std::make_unique<ActionNode>("Move"));
        component.BindAction("Move", [](const BehaviorTreeContext& context) {
            const int ticks = context.blackboard.GetOr<int>("ticks", 0);
            context.blackboard.Set("ticks", ticks + 1);
            const bool accepted = context.commands.RequestMoveTo({context.deltaTime, 0.0f, 0.0f});
            assert(accepted);
            return BehaviorStatus::Running;
        });
    };
    configure(first);
    configure(second);
    auto& a = *first.GetComponent<BehaviorTreeComponent>();
    auto& b = *second.GetComponent<BehaviorTreeComponent>();
    a.GetBlackboard().Set("ticks", 5);
    manager.Update(0.25f);
    assert(first.GetTransform().GetWorldPosition().x == 0.0f); // 判断だけでは動かしません。
    manager.LateUpdate(0.25f);
    assert(a.GetBlackboard().GetOr<int>("ticks", 0) == 6);
    assert(b.GetBlackboard().GetOr<int>("ticks", 0) == 1);
    assert(first.GetTransform().GetWorldPosition().x == 0.25f);
    assert(a.GetStatus() == BehaviorStatus::Running);

    int legacyCalls = 0;
    a.SetTree(std::make_unique<ActionNode>(
        "Legacy", [&](TakeC::Blackboard& blackboard) {
            ++legacyCalls;
            blackboard.Set("legacy", true);
            return BehaviorStatus::Success;
        }));
    manager.Update(0.1f);
    assert(legacyCalls == 1);
    assert(a.GetBlackboard().GetOr<bool>("legacy", false));
}

template<class Composite>
void TestContextPropagation() {
    GameObject object("Nested");
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    auto root = std::make_unique<Composite>();
    auto sequence = std::make_unique<SequenceNode>();
    sequence->AddChild(std::make_unique<WaitNode>(0.5f));
    sequence->AddChild(std::make_unique<ActionNode>("Reached"));
    root->AddChild(std::move(sequence));
    int calls = 0;
    component.BindAction("Reached", [&](const BehaviorTreeContext& context) {
        assert(&context.owner == &object);
        assert(&context.blackboard == &component.GetBlackboard());
        assert(context.deltaTime == 0.25f);
        ++calls;
        return BehaviorStatus::Success;
    });
    component.SetRepeat(false);
    component.SetTree(std::move(root));
    object.Update(0.25f);
    assert(component.GetStatus() == BehaviorStatus::Running);
    assert(calls == 0);
    object.Update(0.25f);
    assert(component.GetStatus() == BehaviorStatus::Success);
    assert(component.GetTree()->GetCurrentStatus() == BehaviorStatus::Success);
    assert(calls == 1);
    object.Update(0.25f);
    assert(calls == 1);
    component.Restart();
    assert(component.GetTree()->GetCurrentStatus() == BehaviorStatus::Invalid);
}

void TestPauseDisableStopAndBlackboardWait() {
    GameObject object;
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    component.GetBlackboard().Set("duration", 1.0f);
    component.SetTree(std::make_unique<WaitBlackboardTimeNode>("duration"));
    component.SetRepeat(false);
    object.Update(0.25f);
    component.Pause();
    object.Update(100.0f);
    component.Resume();
    component.SetEnabled(false);
    object.Update(100.0f);
    component.SetEnabled(true);
    object.SetActive(false);
    object.Update(100.0f);
    object.SetActive(true);
    object.FixedUpdate(100.0f);
    object.LateUpdate(100.0f);
    object.Update(0.25f);
    assert(component.GetStatus() == BehaviorStatus::Running);
    component.Stop();
    assert(!component.IsPlaying());
    assert(component.GetStatus() == BehaviorStatus::Invalid);
    assert(component.GetBlackboard().GetOr<float>("duration", 0.0f) == 1.0f);
    component.Resume();
    for (int i = 0; i < 3; ++i) {
        object.Update(0.25f);
        assert(component.GetStatus() == BehaviorStatus::Running);
    }
    object.Update(0.25f);
    assert(component.GetStatus() == BehaviorStatus::Success);
}

void TestRepeatFailureAndReplacement() {
    int resets = 0;
    GameObject object;
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    component.BindAction("Fail", [](const BehaviorTreeContext&) {
        return BehaviorStatus::Failure;
    }, [&] { ++resets; });
    component.SetTree(std::make_unique<ActionNode>("Fail"));
    object.Update(0.1f);
    assert(component.GetStatus() == BehaviorStatus::Failure && component.IsPlaying());
    object.Update(0.1f);
    assert(resets == 1);
    component.SetTree(std::make_unique<ActionNode>("Missing"));
    assert(resets == 2);
    component.SetRepeat(false);
    object.Update(0.1f);
    assert(component.GetStatus() == BehaviorStatus::Failure && !component.IsPlaying());
    component.SetTree(nullptr);
    object.Update(0.1f);
    assert(component.GetTree() == nullptr);
    assert(component.GetStatus() == BehaviorStatus::Invalid);
}

void TestDestroyAndReentrantMutation() {
    int resets = 0;
    TakeC::GameObjectManager manager;
    auto& object = manager.CreateObject("Destroy");
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    component.BindAction("Destroy", [&](const BehaviorTreeContext& context) {
        bool rejected = false;
        try {
            component.SetTree(nullptr);
        } catch (const std::logic_error&) {
            rejected = true;
        }
        assert(rejected);
        context.owner.Destroy();
        return BehaviorStatus::Running;
    }, [&] { ++resets; });
    component.SetTree(std::make_unique<ActionNode>("Destroy"));
    manager.Update(0.1f);
    assert(manager.GetObjectCount() == 0);
    assert(resets == 1);
}

void TestJsonAssetAndCustomNode() {
    const auto json = nlohmann::json::parse(R"({
        "formatVersion": 1, "name": "ComponentTest",
        "root": {
            "nodeType": "SEQUENCE", "children": [
                {"nodeType": "WAIT", "waitTime": 0.5},
                {"nodeType": "ACTION", "targetState": "Test.Attack"}
            ]
        }
    })");
    const auto asset = json.get<BehaviorTreeAsset>();
    TakeC::BehaviorTreeFactory factory;
    int calls = 0;
    GameObject target("Target");
    GameObject object;
    class AttackHandler final : public TakeC::AICommandHandlerComponent {
    public:
        AttackHandler(int& calls, TakeC::GameObjectId targetId) : calls_(calls), targetId_(targetId) {}
        bool HandleCommand(const TakeC::AICommand& command) override {
            const auto* attack = std::get_if<TakeC::AIAttackCommand>(&command);
            if (!attack) { return false; }
            assert(attack->targetId == targetId_);
            ++calls_;
            return true;
        }
    private:
        int& calls_;
        TakeC::GameObjectId targetId_;
    };
    object.AddComponent<AttackHandler>(calls, target.GetId());
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    component.SetTree(factory.BuildBehaviorTree(asset));
    assert(component.GetTree() != nullptr);
    component.BindAction("Test.Attack", [&](const BehaviorTreeContext& context) {
        assert(&context.owner == &object);
        return context.commands.RequestAttack(target.GetId()) ?
            BehaviorStatus::Success : BehaviorStatus::Failure;
    });
    object.Update(0.25f);
    assert(calls == 0);
    object.Update(0.25f);
    assert(calls == 0);
    object.LateUpdate(0.25f);
    assert(calls == 1);
    object.LateUpdate(0.25f);
    assert(calls == 1); // 攻撃要求を二重実行しません。

    class CustomNode final : public BehaviorNode {
    public:
        BehaviorStatus Execute(TakeC::Blackboard& blackboard) override {
            blackboard.Set("custom", true);
            return BehaviorStatus::Success;
        }
    };
    component.SetTree(std::make_unique<CustomNode>());
    object.Update(0.1f);
    assert(component.GetBlackboard().GetOr<bool>("custom", false));
}

template<class Composite>
void TestAbortPropagation() {
    GameObject object;
    auto& component = object.AddComponent<BehaviorTreeComponent>();
    auto root = std::make_unique<Composite>();
    root->AddChild(std::make_unique<ActionNode>("Abort"));
    component.BindAction("Abort", [](const BehaviorTreeContext&) {
        return BehaviorStatus::Aborted;
    });
    component.SetTree(std::move(root));
    object.Update(0.1f);
    assert(component.GetStatus() == BehaviorStatus::Aborted);
    assert(!component.IsPlaying());
    object.Update(0.1f);
    assert(component.GetStatus() == BehaviorStatus::Aborted);
}

} // namespace

int main() {
    RunAICommandBufferTests();
    TestCommonControllerLifecycleAndBrainReplacement();
    TestGenericControllerBehaviorTreeAndIsolation();
    TestControllerTickIntervalAndValidation();
    TestCommonControllerReentrantProtection();
    TestInstanceIsolationAndLegacyNodes();
    TestContextPropagation<SequenceNode>();
    TestContextPropagation<SelectorNode>();
    TestContextPropagation<PlannerSelectorNode>();
    TestContextPropagation<WeightSelectorNode>();
    TestPauseDisableStopAndBlackboardWait();
    TestRepeatFailureAndReplacement();
    TestDestroyAndReentrantMutation();
    TestJsonAssetAndCustomNode();
    TestAbortPropagation<SequenceNode>();
    TestAbortPropagation<SelectorNode>();
    TestAbortPropagation<PlannerSelectorNode>();
    TestAbortPropagation<WeightSelectorNode>();
    return 0;
}
