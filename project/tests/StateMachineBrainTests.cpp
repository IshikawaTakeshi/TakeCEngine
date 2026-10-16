#include "engine/AI/StateMachineBrain.h"
#include "engine/GameObject/AIControllerComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/BehaviorTree/BehaviorTreeBrain.h"
#include "examples/StateMachineBrainExample.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace {
using namespace TakeC;
static_assert(std::is_base_of_v<IAgentBrain, StateMachineBrain>);

struct Fixture {
    GameObject owner;
    Blackboard blackboard;
    AICommandBuffer commands;
    AIContext context{owner, blackboard, commands, 0.25f};
};

class TestState final : public IAgentState {
public:
    using Callback = std::function<void(AIContext&)>;
    TestState(std::string name, std::vector<std::string>& trace)
        : name_(std::move(name)), trace_(trace) {}
    void Enter(AIContext& context) override {
        trace_.push_back(name_ + ".Enter");
        if (onEnter) { onEnter(context); }
    }
    void Tick(AIContext& context) override {
        trace_.push_back(name_ + ".Tick");
        if (onTick) { onTick(context); }
    }
    void Exit(AIContext& context) override {
        trace_.push_back(name_ + ".Exit");
        if (onExit) { onExit(context); }
    }
    Callback onEnter;
    Callback onTick;
    Callback onExit;
private:
    std::string name_;
    std::vector<std::string>& trace_;
};

TestState& AddState(StateMachineBrain& brain, const std::string& name,
    std::vector<std::string>& trace) {
    auto state = std::make_unique<TestState>(name, trace);
    auto& result = *state;
    brain.AddState(name, std::move(state));
    return result;
}

template<class Exception, class Callback>
void ExpectThrows(Callback callback) {
    bool rejected = false;
    try { callback(); } catch (const Exception&) { rejected = true; }
    assert(rejected);
}

bool Near(float a, float b) { return std::abs(a - b) < 0.0001f; }

void TestTransitionOrderAndOneTransitionPerTick() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    AddState(brain, "A", trace);
    AddState(brain, "B", trace);
    AddState(brain, "C", trace);
    brain.SetInitialState("A");
    brain.AddTransition("A", "B", [](const AIContext& context) {
        return context.blackboard.GetOr<bool>("ready", false);
    });
    brain.AddTransition("B", "C", [](const AIContext&) { return true; });
    brain.Start(fixture.context);
    brain.Start(fixture.context);
    brain.Tick(fixture.context);
    assert((trace == std::vector<std::string>{"A.Enter", "A.Tick"}));
    fixture.blackboard.Set("ready", true);
    trace.clear();
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B");
    assert((trace == std::vector<std::string>{"A.Exit", "B.Enter", "B.Tick"}));
    trace.clear();
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "C");
    assert((trace == std::vector<std::string>{"B.Exit", "C.Enter", "C.Tick"}));
    assert(!brain.IsFinished());
}

void TestPrioritiesRequestsAndRegistrationOrder() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    for (const std::string name : {"A", "B", "C", "D"}) { AddState(brain, name, trace); }
    brain.SetInitialState("A");
    brain.AddTransition("A", "B", [](const AIContext&) { return true; }, 5);
    brain.AddTransition("A", "C", [](const AIContext&) { return true; }, 5);
    brain.AddTransition("A", "D", [](const AIContext&) { return true; }, -1);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B"); // 同priorityは登録順。

    brain.Reset(fixture.context);
    brain.RequestTransition("D", 5);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "D"); // 同priorityは明示要求。
    brain.Reset(fixture.context);
    brain.RequestTransition("D", 4);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B"); // 条件のpriorityが高い。
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B"); // 条件に負けた明示要求を持ち越しません。
    brain.Reset(fixture.context);
    brain.RequestTransition("D", 9);
    brain.RequestTransition("C", 8);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "D"); // 低priorityの後発要求は無視。
    brain.Reset(fixture.context);
    brain.RequestTransition("B", 9);
    brain.RequestTransition("C", 9);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "C"); // 同priorityの明示要求は最後。

    StateMachineBrain negative;
    AddState(negative, "A", trace);
    AddState(negative, "B", trace);
    AddState(negative, "C", trace);
    negative.SetInitialState("A");
    negative.AddTransition("A", "B", [](const AIContext&) { return true; },
        std::numeric_limits<int>::min());
    negative.AddTransition("A", "C", [](const AIContext&) { return true; }, -1);
    negative.Tick(fixture.context);
    assert(negative.GetCurrentStateId() == "C");
}

void TestDeferredRequestsFromEnterTickAndExit() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    auto& a = AddState(brain, "A", trace);
    auto& b = AddState(brain, "B", trace);
    AddState(brain, "C", trace);
    AddState(brain, "D", trace);
    brain.SetInitialState("A");
    a.onTick = [&](AIContext&) { brain.RequestTransition("B"); };
    a.onExit = [&](AIContext&) { brain.RequestTransition("D", 1); };
    b.onEnter = [&](AIContext&) { brain.RequestTransition("C", 2); };
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "A");
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B"); // Exit/Enterの要求を同じTickで実行しない。
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "C");
    assert((trace == std::vector<std::string>{
        "A.Enter", "A.Tick", "A.Exit", "B.Enter", "B.Tick", "B.Exit", "C.Enter", "C.Tick"}));

    StateMachineBrain fromStart;
    auto& initial = AddState(fromStart, "A", trace);
    AddState(fromStart, "B", trace);
    fromStart.SetInitialState("A");
    initial.onEnter = [&](AIContext&) { fromStart.RequestTransition("B"); };
    fromStart.Start(fixture.context);
    assert(fromStart.GetCurrentStateId() == "A");
    fromStart.Tick(fixture.context);
    assert(fromStart.GetCurrentStateId() == "B"); // Start後の最初のTickが予約実行時点。
    fromStart.Reset(fixture.context);
    fromStart.Tick(fixture.context);
    assert(fromStart.GetCurrentStateId() == "A"); // このTickでの初回Enter要求は次回へ予約。
    fromStart.Tick(fixture.context);
    assert(fromStart.GetCurrentStateId() == "B");
}

void TestResetKeepsDefinitionsAndDiscardsExitRequests() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    auto& a = AddState(brain, "A", trace);
    AddState(brain, "B", trace);
    brain.SetInitialState("A");
    a.onExit = [&](AIContext&) { brain.RequestTransition("B", 100); };
    fixture.blackboard.Set("persistent", 42);
    brain.Reset(fixture.context);
    assert(trace.empty());
    brain.Start(fixture.context);
    brain.RequestTransition("B");
    brain.Reset(fixture.context);
    brain.Reset(fixture.context);
    assert(!brain.IsActive() && brain.GetCurrentStateId().empty());
    assert((trace == std::vector<std::string>{"A.Enter", "A.Exit"}));
    brain.Tick(fixture.context); // ControllerはRestart時にStartを再呼び出しません。
    assert(brain.GetCurrentStateId() == "A");
    assert(brain.GetStateCount() == 2);
    assert(fixture.blackboard.GetOr<int>("persistent", 0) == 42);
    brain.RequestTransition("A", 100);
    trace.clear();
    brain.Tick(fixture.context);
    assert((trace == std::vector<std::string>{"A.Tick"})); // 同一State要求はno-op。
    brain.SetInitialState("B");
    assert(brain.GetCurrentStateId() == "A");
    brain.Reset(fixture.context);
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "B");
}

void TestValidationAndDeltaTime() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    ExpectThrows<std::logic_error>([&] { brain.Start(fixture.context); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddState("", std::make_unique<TestState>("A", trace)); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddState("A", nullptr); });
    AddState(brain, "A", trace);
    AddState(brain, "B", trace);
    ExpectThrows<std::invalid_argument>([&] { brain.AddState("A", std::make_unique<TestState>("A", trace)); });
    ExpectThrows<std::invalid_argument>([&] { brain.SetInitialState("Missing"); });
    ExpectThrows<std::invalid_argument>([&] { brain.RequestTransition("Missing"); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddTransition("A", "Missing", [](const AIContext&) { return true; }); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddTransition("Missing", "B", [](const AIContext&) { return true; }); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddTransition("A", "B", {}); });
    ExpectThrows<std::invalid_argument>([&] { brain.AddTransition("A", "A", [](const AIContext&) { return true; }); });
    assert(brain.GetStateCount() == 2 && brain.GetTransitionCount() == 0);
    brain.SetInitialState("A");
    brain.Start(fixture.context);
    for (float invalid : {-1.0f, std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()}) {
        fixture.context.deltaTime = invalid;
        ExpectThrows<std::invalid_argument>([&] { brain.Tick(fixture.context); });
        ExpectThrows<std::invalid_argument>([&] { brain.Start(fixture.context); });
        assert(brain.GetCurrentStateId() == "A");
    }
    fixture.context.deltaTime = 0.0f;
    brain.Tick(fixture.context);
    ExpectThrows<std::invalid_argument>([&] { brain.RequestTransition("Missing"); });
    assert(brain.GetCurrentStateId() == "A");
}

void TestReentrantProtectionAndConditionPurity() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    auto& a = AddState(brain, "A", trace);
    AddState(brain, "B", trace);
    brain.SetInitialState("A");
    const auto expectMutationsRejected = [&](AIContext& context) {
        ExpectThrows<std::logic_error>([&] { brain.AddState("C", std::make_unique<TestState>("C", trace)); });
        ExpectThrows<std::logic_error>([&] { brain.SetInitialState("B"); });
        ExpectThrows<std::logic_error>([&] { brain.AddTransition("A", "B", [](const AIContext&) { return true; }); });
        ExpectThrows<std::logic_error>([&] { brain.Start(context); });
        ExpectThrows<std::logic_error>([&] { brain.Tick(context); });
        ExpectThrows<std::logic_error>([&] { brain.Reset(context); });
    };
    a.onEnter = expectMutationsRejected;
    a.onTick = expectMutationsRejected;
    a.onExit = expectMutationsRejected;
    brain.Tick(fixture.context);
    brain.Reset(fixture.context);
    bool attemptMutation = true;
    brain.AddTransition("A", "B", [&](const AIContext&) {
        if (attemptMutation) { brain.RequestTransition("B"); }
        return false;
    });
    ExpectThrows<std::logic_error>([&] { brain.Tick(fixture.context); });
    assert(!brain.IsActive()); // 条件失敗でもGuardを解放し、StateをExit。
    attemptMutation = false;
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "A");
}

void TestCallbackFailureCleanup() {
    Fixture fixture;
    std::vector<std::string> trace;
    StateMachineBrain brain;
    auto& a = AddState(brain, "A", trace);
    auto& b = AddState(brain, "B", trace);
    AddState(brain, "C", trace);
    brain.SetInitialState("A");
    b.onEnter = [&](AIContext&) {
        brain.RequestTransition("C");
        throw std::runtime_error("Enter failed");
    };
    brain.Start(fixture.context);
    brain.RequestTransition("B");
    trace.clear();
    ExpectThrows<std::runtime_error>([&] { brain.Tick(fixture.context); });
    assert(!brain.IsActive());
    assert((trace == std::vector<std::string>{"A.Exit", "B.Enter", "B.Exit"}));
    brain.Reset(fixture.context);
    trace.clear();
    brain.Tick(fixture.context);
    assert(brain.GetCurrentStateId() == "A"); // 失敗したEnterの予約Cも消去。
    a.onTick = [](AIContext&) { throw std::runtime_error("Tick failed"); };
    trace.clear();
    ExpectThrows<std::runtime_error>([&] { brain.Tick(fixture.context); });
    assert((trace == std::vector<std::string>{"A.Tick", "A.Exit"}));
    assert(!brain.IsActive());

    a.onTick = {};
    a.onExit = [](AIContext&) { throw std::runtime_error("Exit failed"); };
    brain.Start(fixture.context);
    trace.clear();
    ExpectThrows<std::runtime_error>([&] { brain.Reset(fixture.context); });
    brain.Reset(fixture.context); // 契約違反のExit例外でも二重Exitしません。
    assert((trace == std::vector<std::string>{"A.Exit"}));
}

void TestControllerLifecycleCommandsAndBrainSwap() {
    std::vector<std::string> trace;
    GameObject object;
    auto& controller = object.AddComponent<AIControllerComponent>();
    auto& movement = object.AddComponent<AIMovementComponent>();
    auto brain = std::make_unique<StateMachineBrain>();
    auto* fsm = brain.get();
    auto& a = AddState(*brain, "A", trace);
    AddState(*brain, "B", trace);
    brain->SetInitialState("A");
    a.onEnter = [&](AIContext& context) {
        assert(&context.owner == &object);
        assert(&context.blackboard == &controller.GetBlackboard());
        assert(&context.commands == &controller.GetCommandBuffer());
        assert(context.commands.RequestMoveTo({10.0f, 0.0f, 0.0f}));
    };
    a.onTick = [&](AIContext& context) {
        assert(context.deltaTime == 0.5f);
        context.blackboard.Set("ticks", context.blackboard.GetOr<int>("ticks", 0) + 1);
    };
    a.onExit = [](AIContext& context) { context.commands.RequestStopMovement(); };
    controller.SetBrain(std::move(brain));
    controller.SetTickInterval(0.5f);
    object.Update(0.25f);
    assert((trace == std::vector<std::string>{"A.Enter"})); // StartはTick間隔とは独立。
    object.LateUpdate(0.25f);
    assert(movement.IsMoving() && Near(object.GetTransform().GetWorldPosition().x, 0.25f));
    controller.Pause();
    object.Update(10.0f);
    controller.Resume();
    object.Update(0.25f);
    assert((trace == std::vector<std::string>{"A.Enter", "A.Tick"}));
    fsm->RequestTransition("B");
    controller.Restart();
    assert(!fsm->IsActive() && !movement.IsMoving() && controller.GetCommandBuffer().IsEmpty());
    object.Update(0.25f);
    assert(fsm->GetCurrentStateId().empty());
    object.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "A");
    assert(controller.GetBlackboard().GetOr<int>("ticks", 0) == 2);
    assert((trace == std::vector<std::string>{"A.Enter", "A.Tick", "A.Exit", "A.Enter", "A.Tick"}));

    auto tree = std::make_unique<BehaviorTreeBrain>();
    tree->SetTree(std::make_unique<ActionNode>("Legacy", [](Blackboard& blackboard) {
        blackboard.Set("tree", true);
        return BehaviorStatus::Success;
    }));
    controller.SetBrain(std::move(tree)); // 旧FSM.Exitを呼び、同じControllerでBTへ交換。
    assert(trace.back() == "A.Exit");
    object.Update(0.5f);
    assert(controller.GetBlackboard().GetOr<bool>("tree", false));
    assert(controller.GetBlackboard().GetOr<int>("ticks", 0) == 2);
}

void TestInstanceIsolationAndDetach() {
    class CounterState final : public IAgentState {
    public:
        explicit CounterState(int& exits) : exits_(exits) {}
        void Enter([[maybe_unused]] AIContext& context) override { ticks_ = 0; }
        void Tick(AIContext& context) override { context.blackboard.Set("ticks", ++ticks_); }
        void Exit([[maybe_unused]] AIContext& context) override { ++exits_; }
    private:
        int& exits_;
        int ticks_ = 0;
    };
    int firstExits = 0;
    int secondExits = 0;
    {
        GameObjectManager objects;
        auto& first = objects.CreateObject();
        auto& second = objects.CreateObject();
        const auto configure = [](GameObject& object, int& exits) -> AIControllerComponent& {
            auto& controller = object.AddComponent<AIControllerComponent>();
            auto brain = std::make_unique<StateMachineBrain>();
            brain->AddState("A", std::make_unique<CounterState>(exits));
            brain->SetInitialState("A");
            controller.SetBrain(std::move(brain));
            return controller;
        };
        auto& a = configure(first, firstExits);
        auto& b = configure(second, secondExits);
        first.Update(0.25f);
        first.Update(0.25f);
        second.Update(0.25f);
        assert(a.GetBlackboard().GetOr<int>("ticks", 0) == 2);
        assert(b.GetBlackboard().GetOr<int>("ticks", 0) == 1);
        a.Stop();
        first.Update(10.0f);
        assert(firstExits == 1);
        a.Resume();
        first.Update(0.25f);
        assert(a.GetBlackboard().GetOr<int>("ticks", 0) == 1);
        first.Destroy();
        objects.FlushPendingChanges();
        assert(firstExits == 2);
    }
    assert(firstExits == 2 && secondExits == 1);
}

void TestPatrolChaseAttackExample() {
    class Combat final : public AICommandHandlerComponent {
    public:
        Combat(GameObjectManager& objects, int& attacks) : objects_(objects), attacks_(attacks) {}
        bool HandleCommand(const AICommand& command) override {
            const auto* attack = std::get_if<AIAttackCommand>(&command);
            if (!attack) { return false; }
            const auto* target = objects_.FindObject(attack->targetId);
            if (target && target->IsActive() && !target->IsDestroyRequested()) { ++attacks_; }
            return true;
        }
    private:
        GameObjectManager& objects_;
        int& attacks_;
    };
    int attacks = 0;
    GameObjectManager objects;
    auto& target = objects.CreateObject("Target");
    target.GetTransform().SetLocalPosition({5.0f, 0.0f, 0.0f});
    auto& enemy = objects.CreateObject("Enemy");
    auto& controller = enemy.AddComponent<AIControllerComponent>();
    auto& movement = enemy.AddComponent<AIMovementComponent>();
    movement.SetSpeed(2.0f);
    enemy.AddComponent<Combat>(objects, attacks);
    auto brain = Examples::BuildEnemyBrain(objects, target.GetId(), {-5.0f, 0.0f, 0.0f});
    auto* fsm = brain.get();
    controller.SetBrain(std::move(brain));
    objects.Update(0.25f);
    objects.LateUpdate(0.25f);
    assert(fsm->GetCurrentStateId() == "Patrol");
    assert(Near(enemy.GetTransform().GetWorldPosition().x, -0.5f));

    controller.GetBlackboard().Set("enemyVisible", true);
    objects.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Chase");
    objects.LateUpdate(0.25f);
    assert(Near(enemy.GetTransform().GetWorldPosition().x, 0.0f));
    controller.GetBlackboard().Set("inAttackRange", true);
    objects.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Attack" && attacks == 0);
    for (const auto& command : controller.GetCommandBuffer().GetCommands()) {
        assert(!std::holds_alternative<AIMoveToCommand>(command)); // 旧Chase.Tickは実行しない。
    }
    objects.LateUpdate(0.25f);
    assert(attacks == 1 && !movement.IsMoving());
    objects.Update(0.25f);
    objects.LateUpdate(0.25f);
    assert(attacks == 1); // Enterでの単発攻撃はTickで繰り返さない。

    controller.GetBlackboard().Set("inAttackRange", false);
    objects.Update(0.25f);
    objects.LateUpdate(0.25f);
    assert(fsm->GetCurrentStateId() == "Chase");
    assert(Near(enemy.GetTransform().GetWorldPosition().x, 0.5f));
    target.Destroy();
    objects.Update(0.25f);
    objects.LateUpdate(0.25f);
    assert(!movement.IsMoving());
    objects.Update(0.25f);
    objects.LateUpdate(0.25f);
    assert(fsm->GetCurrentStateId() == "Patrol" && attacks == 1);
}

void TestSensedEnemyPipeline() {
    /// <summary>有効な対象への攻撃要求だけを消費するゲーム側Handlerです。</summary>
    class Combat final : public AICommandHandlerComponent {
    public:
        Combat(GameObjectManager& objects, int& attacks) : objects_(objects), attacks_(attacks) {}
        bool HandleCommand(const AICommand& command) override {
            const auto* attack = std::get_if<AIAttackCommand>(&command);
            if (!attack) { return false; }
            const auto* target = objects_.FindObject(attack->targetId);
            if (target && target->IsActive() && !target->IsDestroyRequested()) { ++attacks_; }
            return true;
        }
    private:
        GameObjectManager& objects_;
        int& attacks_;
    };

    GameObjectManager world;
    auto& enemy = world.CreateObject("SensedEnemy");
    auto& controller = enemy.AddComponent<AIControllerComponent>();
    auto& movement = enemy.AddComponent<AIMovementComponent>();
    movement.SetSpeed(2.0f);
    int attacks = 0;
    enemy.AddComponent<Combat>(world, attacks);
    auto& sensor = enemy.AddComponent<SensorComponent>(); // Controllerより後でもPerceptionが先。
    auto& target = world.CreateObject("SensorTarget"); // Enemyより後の対象もIDで解決する。
    const GameObjectId targetId = target.GetId();
    target.GetTransform().SetLocalPosition({10.0f, 0.0f, 0.0f});
    ExpectThrows<std::invalid_argument>([&] {
        Examples::ConfigureDistanceSensor(sensor, targetId, 1.0f, 2.0f);
    });
    assert(sensor.GetSensorCount() == 0);
    Examples::ConfigureDistanceSensor(sensor, targetId, 6.0f, 1.0f);
    auto brain = Examples::BuildSensedEnemyBrain();
    auto* fsm = brain.get();
    controller.SetBrain(std::move(brain));

    world.Update(0.25f);
    world.LateUpdate(0.25f);
    const auto& blackboard = controller.GetBlackboard();
    assert(fsm->GetCurrentStateId() == "Idle");
    assert(!blackboard.GetOr(Examples::SensedEnemyKeys::kVisible, true));
    assert(!blackboard.Contains(Examples::SensedEnemyKeys::kTargetId.name));
    assert(!movement.IsMoving() && attacks == 0);

    target.GetTransform().SetLocalPosition({5.0f, 0.0f, 0.0f});
    world.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Chase");
    assert(blackboard.GetOr(Examples::SensedEnemyKeys::kTargetId, kInvalidGameObjectId) == targetId);
    assert(Near(blackboard.GetOr(Examples::SensedEnemyKeys::kTargetDistance, 0.0f), 5.0f));
    world.LateUpdate(0.25f);
    assert(Near(enemy.GetTransform().GetWorldPosition().x, 0.5f));

    target.GetTransform().SetLocalPosition({1.0f, 0.0f, 0.0f});
    world.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Attack" && attacks == 0);
    assert(blackboard.GetOr(Examples::SensedEnemyKeys::kInAttackRange, false));
    world.LateUpdate(0.25f);
    assert(attacks == 1 && !movement.IsMoving());
    world.Update(0.25f);
    world.LateUpdate(0.25f);
    assert(attacks == 1); // Attack.Enterだけが攻撃する。

    target.GetTransform().SetLocalPosition({5.0f, 0.0f, 0.0f});
    world.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Chase");
    world.LateUpdate(0.25f);
    assert(Near(enemy.GetTransform().GetWorldPosition().x, 1.0f));

    target.Destroy();
    world.Update(0.25f);
    assert(fsm->GetCurrentStateId() == "Idle");
    assert(!blackboard.Contains(Examples::SensedEnemyKeys::kTargetId.name));
    assert(!blackboard.Contains(Examples::SensedEnemyKeys::kTargetPosition.name));
    world.LateUpdate(0.25f);
    assert(!movement.IsMoving() && attacks == 1);
}

void TestControllerDropsPartialFailureCommands() {
    std::vector<std::string> trace;
    GameObject object;
    auto& controller = object.AddComponent<AIControllerComponent>();
    auto brain = std::make_unique<StateMachineBrain>();
    auto* fsm = brain.get();
    auto& a = AddState(*brain, "A", trace);
    brain->SetInitialState("A");
    a.onTick = [](AIContext& context) {
        assert(context.commands.RequestAttack(context.owner.GetId()));
        throw std::runtime_error("State Tick failed");
    };
    controller.SetBrain(std::move(brain));
    ExpectThrows<std::runtime_error>([&] { object.Update(0.25f); });
    assert(!fsm->IsActive() && controller.GetCommandBuffer().IsEmpty());
    assert((trace == std::vector<std::string>{"A.Enter", "A.Tick", "A.Exit"}));
}

void TestControllerReceivesWorldContext() {
    GameObjectManager world;
    auto& owner = world.CreateObject("WorldContext");
    auto& controller = owner.AddComponent<AIControllerComponent>();
    std::vector<std::string> trace;
    auto brain = std::make_unique<StateMachineBrain>();
    auto& state = AddState(*brain, "Observe", trace);
    brain->SetInitialState("Observe");
    state.onEnter = [&](AIContext& context) {
        assert(context.world == &world);
        assert(context.services == &world.GetServices());
    };
    state.onTick = state.onEnter;
    controller.SetBrain(std::move(brain));
    world.Update(0.1f);
    assert((trace == std::vector<std::string>{"Observe.Enter", "Observe.Tick"}));
}

} // namespace

void RunStateMachineBrainTests() {
    TestTransitionOrderAndOneTransitionPerTick();
    TestPrioritiesRequestsAndRegistrationOrder();
    TestDeferredRequestsFromEnterTickAndExit();
    TestResetKeepsDefinitionsAndDiscardsExitRequests();
    TestValidationAndDeltaTime();
    TestReentrantProtectionAndConditionPurity();
    TestCallbackFailureCleanup();
    TestControllerLifecycleCommandsAndBrainSwap();
    TestInstanceIsolationAndDetach();
    TestPatrolChaseAttackExample();
    TestSensedEnemyPipeline();
    TestControllerDropsPartialFailureCommands();
    TestControllerReceivesWorldContext();
}
