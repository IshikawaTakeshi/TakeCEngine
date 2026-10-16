#include "engine/AI/AICommandBuffer.h"
#include "engine/GameObject/BehaviorTreeComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/CharacterMotorComponent.h"
#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/NavigationAgentComponent.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Navigation/IPathfindingService.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
using namespace TakeC;

bool Near(float a, float b) { return std::abs(a - b) < 0.0001f; }

class RecordingCombat final : public AICommandHandlerComponent {
public:
    RecordingCombat(GameObjectManager& objects, std::vector<GameObjectId>& attacks)
        : objects_(objects), attacks_(attacks) {}
    bool HandleCommand(const AICommand& command) override {
        const auto* attack = std::get_if<AIAttackCommand>(&command);
        if (!attack) { return false; }
        const auto* target = objects_.FindObject(attack->targetId);
        if (target && !target->IsDestroyRequested() && target->IsActive()) {
            attacks_.push_back(target->GetId());
        }
        return true; // 対象消失による実行拒否も消費し、再実行しません。
    }
private:
    GameObjectManager& objects_;
    std::vector<GameObjectId>& attacks_;
};

void TestBufferOrderingCapacityAndIsolation() {
    AICommandBuffer buffer(3);
    AICommandBuffer other;
    assert(buffer.RequestMoveTo({1.0f, 0.0f, 0.0f}));
    assert(buffer.RequestAttack(11, "Primary"));
    assert(buffer.RequestAttack(12, "Secondary"));
    assert(!buffer.RequestAttack(13));
    assert(buffer.RequestMoveTo({2.0f, 0.0f, 0.0f})); // 満杯でも移動要求は置換できます。
    assert(buffer.GetSize() == 3 && other.IsEmpty());
    assert(std::get<AIAttackCommand>(buffer.GetCommands()[0]).targetId == 11);
    assert(std::get<AIAttackCommand>(buffer.GetCommands()[1]).actionId == "Secondary");
    assert(buffer.RequestStopMovement());
    const auto batch = buffer.ConsumeAll();
    assert(batch.size() == 3 && buffer.IsEmpty());
    assert(std::holds_alternative<AIStopMovementCommand>(batch[2]));
    assert(buffer.ConsumeAll().empty());
    assert(buffer.RequestAttack(11));
    assert(buffer.RequestAttack(12));
    assert(buffer.RequestAttack(13));
    assert(!buffer.RequestMoveTo({1.0f, 0.0f, 0.0f}));
    assert(buffer.GetSize() == 3);
    buffer.Clear();
    assert(buffer.IsEmpty() && buffer.GetCapacity() == 3);
}

void TestInvalidRequestsPreserveBuffer() {
    AICommandBuffer buffer;
    assert(buffer.RequestAttack(11));
    auto expectRejected = [&](auto&& request) {
        bool rejected = false;
        try { request(); } catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected && buffer.GetSize() == 1);
    };
    expectRejected([&] { buffer.RequestAttack(kInvalidGameObjectId); });
    expectRejected([&] { buffer.RequestAttack(12, ""); });
    expectRejected([&] { buffer.RequestMoveTo({0.0f, 0.0f, 0.0f}, -1.0f); });
    for (float invalid : {std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()}) {
        expectRejected([&] { buffer.RequestMoveTo({invalid, 0.0f, 0.0f}); });
        expectRejected([&] { buffer.RequestMoveTo({0.0f, invalid, 0.0f}); });
        expectRejected([&] { buffer.RequestMoveTo({0.0f, 0.0f, invalid}); });
        expectRejected([&] { buffer.RequestMoveTo({0.0f, 0.0f, 0.0f}, invalid); });
    }
    bool rejectedCapacity = false;
    try { AICommandBuffer invalid(0); } catch (const std::invalid_argument&) { rejectedCapacity = true; }
    assert(rejectedCapacity);
}

void TestSlowBrainAndContinuousMovement(bool handlerFirst) {
    GameObject object;
    AIMovementComponent* movement = nullptr;
    if (handlerFirst) { movement = &object.AddComponent<AIMovementComponent>(); }
    auto& brain = object.AddComponent<BehaviorTreeComponent>();
    if (!handlerFirst) { movement = &object.AddComponent<AIMovementComponent>(); }
    movement->SetSpeed(2.0f);
    brain.SetTickInterval(0.5f);
    brain.SetRepeat(false);
    brain.SetTree(std::make_unique<ActionNode>("Move"));
    brain.BindAction("Move", [](const AIContext& context) {
        return context.commands.RequestMoveTo({2.0f, 0.0f, 0.0f}, 0.0f) ?
            BehaviorStatus::Success : BehaviorStatus::Failure;
    });
    object.Update(0.25f);
    object.LateUpdate(0.25f);
    assert(Near(object.GetTransform().GetWorldPosition().x, 0.0f));
    object.Update(0.25f);
    assert(!brain.IsPlaying() && !brain.GetCommandBuffer().IsEmpty());
    object.LateUpdate(0.25f);
    assert(Near(object.GetTransform().GetWorldPosition().x, 0.5f));
    assert(brain.GetCommandBuffer().IsEmpty());
    for (int i = 0; i < 3; ++i) {
        object.Update(0.25f); // Brain完了後も保持した移動先へ毎フレーム動きます。
        object.LateUpdate(0.25f);
    }
    assert(Near(object.GetTransform().GetWorldPosition().x, 2.0f));
    assert(!movement->IsMoving());
    assert(brain.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f}));
    object.LateUpdate(10.0f);
    assert(Near(object.GetTransform().GetWorldPosition().x, 10.0f)); // 通り越しません。
}

void TestStopResetReplacementAndDisable() {
    GameObject object;
    auto& controller = object.AddComponent<BehaviorTreeComponent>();
    auto& movement = object.AddComponent<AIMovementComponent>();
    auto issueMove = [&] {
        assert(controller.GetCommandBuffer().RequestMoveTo({100.0f, 0.0f, 0.0f}));
        object.LateUpdate(0.25f);
        assert(movement.IsMoving());
    };
    issueMove();
    controller.SetEnabled(false);
    const float frozen = object.GetTransform().GetWorldPosition().x;
    object.LateUpdate(100.0f);
    assert(Near(object.GetTransform().GetWorldPosition().x, frozen));
    controller.SetEnabled(true);
    object.SetActive(false);
    object.LateUpdate(100.0f);
    object.SetActive(true);
    assert(Near(object.GetTransform().GetWorldPosition().x, frozen));
    controller.Pause();
    object.LateUpdate(0.25f); // Pauseは判断のみ停止。既に始めた行動は継続。
    assert(Near(object.GetTransform().GetWorldPosition().x, frozen + 0.25f));
    assert(controller.GetCommandBuffer().RequestStopMovement());
    object.LateUpdate(0.25f);
    assert(!movement.IsMoving());

    issueMove();
    assert(controller.GetCommandBuffer().RequestAttack(object.GetId()));
    controller.Stop();
    assert(!movement.IsMoving() && controller.GetCommandBuffer().IsEmpty());
    issueMove();
    controller.Reset();
    assert(!movement.IsMoving());
    issueMove();
    controller.SetBrain(std::make_unique<BehaviorTreeBrain>());
    assert(!movement.IsMoving() && controller.GetCommandBuffer().IsEmpty());
    issueMove();
    controller.SetTree(std::make_unique<ActionNode>("Move"));
    assert(!movement.IsMoving());
    issueMove();
    controller.BindAction("Move", [](const AIContext&) { return BehaviorStatus::Running; });
    assert(!movement.IsMoving());
    movement.SetEnabled(false);
    assert(controller.GetCommandBuffer().RequestMoveTo({100.0f, 0.0f, 0.0f}));
    object.LateUpdate(0.25f);
    assert(controller.GetLastUnhandledCommandCount() == 1);
    assert(controller.GetCommandBuffer().IsEmpty());
}

void TestAttackExactlyOnceAndStaleTarget() {
    std::vector<GameObjectId> attacks;
    GameObjectManager objects;
    auto& attacker = objects.CreateObject("Attacker");
    auto& target = objects.CreateObject("Target");
    const auto targetId = target.GetId();
    auto& controller = attacker.AddComponent<AIControllerComponent>();
    attacker.AddComponent<RecordingCombat>(objects, attacks);
    assert(controller.GetCommandBuffer().RequestAttack(targetId));
    assert(controller.GetCommandBuffer().RequestAttack(targetId, "Secondary"));
    objects.Update(0.25f);
    objects.FixedUpdate(0.25f);
    assert(attacks.empty());
    objects.LateUpdate(0.25f);
    assert(attacks.size() == 2);
    objects.LateUpdate(0.25f);
    assert(attacks.size() == 2);
    assert(controller.GetCommandBuffer().RequestAttack(targetId));
    target.Destroy();
    objects.LateUpdate(0.25f);
    assert(attacks.size() == 2 && objects.FindObject(targetId) == nullptr);
    assert(controller.GetCommandBuffer().RequestAttack(targetId));
    objects.LateUpdate(0.25f);
    assert(attacks.size() == 2 && controller.GetCommandBuffer().IsEmpty());
}

void TestWorldSpaceMovementAndInputValidation() {
    GameObject parent;
    parent.GetTransform().SetLocalPosition({10.0f, 0.0f, 0.0f});
    parent.GetTransform().SetLocalScale({2.0f, 2.0f, 2.0f});
    GameObject child;
    child.GetTransform().SetParent(&parent.GetTransform());
    auto& controller = child.AddComponent<AIControllerComponent>();
    auto& movement = child.AddComponent<AIMovementComponent>();
    movement.SetSpeed(2.0f);
    assert(controller.GetCommandBuffer().RequestMoveTo({12.0f, 0.0f, 0.0f}, 0.0f));
    child.LateUpdate(0.5f);
    assert(Near(child.GetTransform().GetWorldPosition().x, 11.0f));
    assert(Near(child.GetTransform().GetLocalPosition().x, 0.5f));
    child.LateUpdate(0.5f);
    assert(Near(child.GetTransform().GetWorldPosition().x, 12.0f));
    assert(!movement.IsMoving());
    for (float invalid : {-1.0f, std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN()}) {
        bool rejected = false;
        try { movement.SetSpeed(invalid); } catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected && movement.GetSpeed() == 2.0f);
    }
}

void TestNavigationAgentDirectRouteAndStop() {
    GameObjectManager world;
    auto& object = world.CreateObject("DirectNavigation");
    auto& controller = object.AddComponent<AIControllerComponent>();
    auto& navigation = object.AddComponent<NavigationAgentComponent>();
    auto& motor = object.AddComponent<CharacterMotorComponent>();
    motor.SetSpeed(2.0f);
    assert(controller.GetCommandBuffer().RequestMoveTo({2.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(0.5f);
    assert(Near(object.GetTransform().GetWorldPosition().x, 1.0f));
    assert(navigation.GetStatus() == NavigationStatus::Following);
    assert(navigation.GetWaypoints().size() == 1);
    world.LateUpdate(0.5f);
    assert(Near(object.GetTransform().GetWorldPosition().x, 2.0f));
    assert(navigation.GetStatus() == NavigationStatus::Arrived && !navigation.IsMoving());
    assert(controller.GetLastUnhandledCommandCount() == 0);

    assert(controller.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f}));
    world.LateUpdate(0.25f);
    assert(navigation.IsMoving());
    assert(controller.GetCommandBuffer().RequestStopMovement());
    world.LateUpdate(0.25f);
    assert(navigation.GetStatus() == NavigationStatus::Idle && !navigation.GetGoal());
    const float stopped = object.GetTransform().GetWorldPosition().x;
    world.LateUpdate(1.0f);
    assert(Near(object.GetTransform().GetWorldPosition().x, stopped));
}

void TestNavigationAgentServiceWaypointsAndRepath() {
    /// <summary>同期探索の呼び出し回数と探索不能を検証するテスト用サービスです。</summary>
    class MockPathfindingService final : public IPathfindingService {
    public:
        std::optional<std::vector<Vector3>> FindPath(
            const Vector3& start, const Vector3& goal) override {
            ++calls;
            if (blocked) { return std::nullopt; }
            return std::vector<Vector3>{
                {(start.x + goal.x) * 0.5f, 0.0f, 0.0f}, goal};
        }
        int calls = 0;
        bool blocked = false;
    };

    GameObjectManager world;
    auto service = std::make_shared<MockPathfindingService>();
    world.GetServices().Register<IPathfindingService>(service);
    auto& object = world.CreateObject("PathNavigation");
    auto& controller = object.AddComponent<AIControllerComponent>();
    auto& navigation = object.AddComponent<NavigationAgentComponent>();
    auto& motor = object.AddComponent<CharacterMotorComponent>();
    motor.SetSpeed(2.0f);

    assert(controller.GetCommandBuffer().RequestMoveTo({4.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(1.0f);
    assert(service->calls == 1 && navigation.GetWaypoints().size() == 2);
    assert(navigation.GetCurrentWaypointIndex() == 1);
    assert(Near(object.GetTransform().GetWorldPosition().x, 2.0f));
    assert(controller.GetCommandBuffer().RequestMoveTo({4.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(1.0f);
    assert(service->calls == 1 && navigation.GetStatus() == NavigationStatus::Arrived);
    assert(Near(object.GetTransform().GetWorldPosition().x, 4.0f));

    assert(controller.GetCommandBuffer().RequestMoveTo({6.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(0.5f);
    assert(service->calls == 2 && navigation.GetCurrentWaypointIndex() == 1);
    assert(Near(object.GetTransform().GetWorldPosition().x, 5.0f));
    service->blocked = true;
    assert(controller.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(0.5f);
    assert(service->calls == 3 && navigation.GetStatus() == NavigationStatus::NoPath);
    assert(Near(object.GetTransform().GetWorldPosition().x, 5.0f));
    assert(controller.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f}, 0.0f));
    world.LateUpdate(0.5f);
    assert(service->calls == 3); // 同じ失敗経路を毎フレーム探索し直さない。
    service->blocked = false;
    navigation.RequestRepath();
    world.LateUpdate(0.5f);
    assert(service->calls == 4 && navigation.GetStatus() == NavigationStatus::Following);
    controller.Stop();
    assert(navigation.GetStatus() == NavigationStatus::Idle && navigation.GetWaypoints().empty());
}

void TestCustomBrainStartCommandsAndSnapshot() {
    class StartBrain final : public IAgentBrain {
    public:
        void Start(AIContext& context) override {
            assert(context.commands.RequestMoveTo({10.0f, 0.0f, 0.0f}));
        }
        void Tick(AIContext&) override {}
    };
    GameObject object;
    auto& controller = object.AddComponent<AIControllerComponent>();
    auto& movement = object.AddComponent<AIMovementComponent>();
    controller.SetBrain(std::make_unique<StartBrain>());
    controller.SetTickInterval(1.0f);
    object.Update(0.25f);
    object.LateUpdate(0.25f); // Tickの前でもStartの要求を実行できます。
    assert(movement.IsMoving() && Near(object.GetTransform().GetWorldPosition().x, 0.25f));

    class FollowUpHandler final : public AICommandHandlerComponent {
    public:
        FollowUpHandler(AIControllerComponent& controller, int& calls)
            : controller_(controller), calls_(calls) {}
        bool HandleCommand(const AICommand& command) override {
            if (!std::holds_alternative<AIAttackCommand>(command)) { return false; }
            ++calls_;
            if (calls_ == 1) {
                assert(controller_.GetCommandBuffer().RequestAttack(GetOwner().GetId()));
            }
            return true;
        }
    private:
        AIControllerComponent& controller_;
        int& calls_;
    };
    int calls = 0; // Handler破棄まで有効にするため別objectを後に作成。
    GameObject followUp;
    auto& source = followUp.AddComponent<AIControllerComponent>();
    followUp.AddComponent<FollowUpHandler>(source, calls);
    assert(source.GetCommandBuffer().RequestAttack(followUp.GetId()));
    followUp.LateUpdate(0.1f);
    assert(calls == 1 && source.GetCommandBuffer().GetSize() == 1);
    followUp.LateUpdate(0.1f);
    assert(calls == 2 && source.GetCommandBuffer().IsEmpty());
}

void TestMultipleControllerRoutingAndUnhandledCommands() {
    GameObject object;
    auto& first = object.AddComponent<AIControllerComponent>();
    auto& second = object.AddComponent<AIControllerComponent>();
    auto& movement = object.AddComponent<AIMovementComponent>();
    movement.SetCommandSource(&second);
    assert(first.GetCommandBuffer().RequestMoveTo({100.0f, 0.0f, 0.0f}));
    assert(second.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f}));
    object.LateUpdate(0.25f);
    assert(first.GetLastUnhandledCommandCount() == 1);
    assert(second.GetLastUnhandledCommandCount() == 0);
    assert(Near(object.GetTransform().GetWorldPosition().x, 0.25f)); // 2重に移動しません。
    first.Stop();
    assert(movement.IsMoving()); // 別ControllerのResetは行動を中断しません。
    second.Stop();
    assert(!movement.IsMoving());

    GameObject other;
    auto& foreign = other.AddComponent<AIControllerComponent>();
    bool rejected = false;
    try { movement.SetCommandSource(&foreign); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

void TestPartialDecisionFailureDiscardsCommands() {
    class ThrowingBrain final : public IAgentBrain {
    public:
        void Tick(AIContext& context) override {
            assert(context.commands.RequestAttack(context.owner.GetId()));
            throw std::runtime_error("Decision failed.");
        }
    };
    GameObject object;
    auto& controller = object.AddComponent<AIControllerComponent>();
    controller.SetBrain(std::make_unique<ThrowingBrain>());
    bool failed = false;
    try { object.Update(0.25f); } catch (const std::runtime_error&) { failed = true; }
    assert(failed && controller.GetCommandBuffer().IsEmpty());
}

void TestDestroyDuringDispatchAndReentrantMutation() {
    class DestroyHandler final : public AICommandHandlerComponent {
    public:
        DestroyHandler(AIControllerComponent& controller, int& calls)
            : controller_(controller), calls_(calls) {}
        bool HandleCommand(const AICommand&) override {
            ++calls_;
            bool rejected = false;
            try { controller_.Stop(); } catch (const std::logic_error&) { rejected = true; }
            assert(rejected);
            GetOwner().Destroy();
            return true;
        }
    private:
        AIControllerComponent& controller_;
        int& calls_;
    };
    int calls = 0;
    GameObjectManager objects;
    auto& object = objects.CreateObject();
    auto& controller = object.AddComponent<AIControllerComponent>();
    object.AddComponent<DestroyHandler>(controller, calls);
    assert(controller.GetCommandBuffer().RequestAttack(object.GetId()));
    assert(controller.GetCommandBuffer().RequestAttack(object.GetId()));
    objects.LateUpdate(0.25f);
    assert(calls == 1 && objects.GetObjectCount() == 0);
}

} // namespace

void RunAICommandBufferTests() {
    TestBufferOrderingCapacityAndIsolation();
    TestInvalidRequestsPreserveBuffer();
    TestSlowBrainAndContinuousMovement(true);
    TestSlowBrainAndContinuousMovement(false);
    TestStopResetReplacementAndDisable();
    TestAttackExactlyOnceAndStaleTarget();
    TestWorldSpaceMovementAndInputValidation();
    TestNavigationAgentDirectRouteAndStop();
    TestNavigationAgentServiceWaypointsAndRepath();
    TestCustomBrainStartCommandsAndSnapshot();
    TestMultipleControllerRoutingAndUnhandledCommands();
    TestPartialDecisionFailureDiscardsCommands();
    TestDestroyDuringDispatchAndReentrantMutation();
}
