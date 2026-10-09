#pragma once

#include "engine/AI/StateMachineBrain.h"
#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"

#include <stdexcept>

namespace TakeC::Examples {

// ゲーム側の使用例です。engineからこのファイルへ依存させません。
// 視認/射程情報はゲーム側からBlackboardへ設定し、対象は実行時にIDで検索します。
inline void RequireCommand(bool accepted) {
    if (!accepted) {
        throw std::overflow_error("Example FSM command buffer is full.");
    }
}

class PatrolState final : public IAgentState {
public:
    explicit PatrolState(Vector3 destination) : destination_(destination) {}
    void Enter(AIContext& context) override {
        RequireCommand(context.commands.RequestMoveTo(destination_));
    }
    void Tick([[maybe_unused]] AIContext& context) override {}
    void Exit(AIContext& context) override {
        // Exitは例外を投げません。満杯で受付不可ならゲーム側の中断方針に委ねます。
        context.commands.RequestStopMovement();
    }
private:
    Vector3 destination_;
};

class ChaseState final : public IAgentState {
public:
    ChaseState(GameObjectManager& objects, GameObjectId targetId)
        : objects_(objects), targetId_(targetId) {}
    void Enter(AIContext& context) override { UpdateTarget(context); }
    void Tick(AIContext& context) override { UpdateTarget(context); }
    void Exit(AIContext& context) override { context.commands.RequestStopMovement(); }
private:
    void UpdateTarget(AIContext& context) {
        const auto* target = objects_.FindObject(targetId_);
        if (target && target->IsActive() && !target->IsDestroyRequested()) {
            RequireCommand(context.commands.RequestMoveTo(target->GetTransform().GetWorldPosition()));
        } else {
            context.blackboard.Set("enemyVisible", false);
            RequireCommand(context.commands.RequestStopMovement());
        }
    }
    GameObjectManager& objects_;
    GameObjectId targetId_;
};

class AttackState final : public IAgentState {
public:
    AttackState(GameObjectManager& objects, GameObjectId targetId)
        : objects_(objects), targetId_(targetId) {}
    void Enter(AIContext& context) override {
        RequireCommand(context.commands.RequestStopMovement());
        const auto* target = objects_.FindObject(targetId_);
        if (target && target->IsActive() && !target->IsDestroyRequested()) {
            RequireCommand(context.commands.RequestAttack(targetId_));
        } else {
            context.blackboard.Set("enemyVisible", false);
        }
    }
    void Tick([[maybe_unused]] AIContext& context) override {}
private:
    GameObjectManager& objects_;
    GameObjectId targetId_;
};

inline std::unique_ptr<StateMachineBrain> BuildEnemyBrain(
    GameObjectManager& objects, GameObjectId targetId, Vector3 patrolDestination) {
    if (targetId == kInvalidGameObjectId) {
        throw std::invalid_argument("Example FSM requires a target ID.");
    }
    auto brain = std::make_unique<StateMachineBrain>();
    brain->AddState("Patrol", std::make_unique<PatrolState>(patrolDestination));
    brain->AddState("Chase", std::make_unique<ChaseState>(objects, targetId));
    brain->AddState("Attack", std::make_unique<AttackState>(objects, targetId));
    brain->SetInitialState("Patrol");
    brain->AddTransition("Patrol", "Chase", [](const AIContext& context) {
        return context.blackboard.GetOr<bool>("enemyVisible", false);
    });
    brain->AddTransition("Chase", "Patrol", [](const AIContext& context) {
        return !context.blackboard.GetOr<bool>("enemyVisible", false);
    }, 10);
    brain->AddTransition("Chase", "Attack", [](const AIContext& context) {
        return context.blackboard.GetOr<bool>("inAttackRange", false);
    });
    brain->AddTransition("Attack", "Patrol", [](const AIContext& context) {
        return !context.blackboard.GetOr<bool>("enemyVisible", false);
    }, 10);
    brain->AddTransition("Attack", "Chase", [](const AIContext& context) {
        return !context.blackboard.GetOr<bool>("inAttackRange", false);
    });
    return brain;
}

} // namespace TakeC::Examples
