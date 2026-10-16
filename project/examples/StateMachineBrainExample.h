#pragma once

#include "engine/AI/StateMachineBrain.h"
#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/SensorComponent.h"
#include "engine/GameObject/TransformComponent.h"

#include <cmath>
#include <stdexcept>

namespace TakeC::Examples {

// ゲーム側の使用例です。engineからこのファイルへ依存させません。
// 旧例はゲーム側がBlackboardを更新し、新例はSensorComponentが更新します。
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

/// <summary>
/// 距離センサーとFSMの間で共有するBlackboardキーです。
/// </summary>
struct SensedEnemyKeys final {
    static constexpr BlackboardKey<bool> kVisible{"enemyVisible"};
    static constexpr BlackboardKey<bool> kInAttackRange{"inAttackRange"};
    static constexpr BlackboardKey<GameObjectId> kTargetId{"targetId"};
    static constexpr BlackboardKey<Vector3> kTargetPosition{"targetPosition"};
    static constexpr BlackboardKey<float> kTargetDistance{"targetDistance"};
};

/// <summary>
/// センサーが対象を発見するまで移動しない状態です。
/// </summary>
class SensedIdleState final : public IAgentState {
public:
    void Enter(AIContext& context) override {
        RequireCommand(context.commands.RequestStopMovement());
    }
    void Tick([[maybe_unused]] AIContext& context) override {}
    void Exit([[maybe_unused]] AIContext& context) override {}
};

/// <summary>
/// Blackboardに記録された対象位置への移動を要求する状態です。
/// </summary>
class SensedChaseState final : public IAgentState {
public:
    void Enter(AIContext& context) override { RequestDestination(context); }
    void Tick(AIContext& context) override { RequestDestination(context); }
    void Exit(AIContext& context) override {
        context.commands.RequestStopMovement();
    }

private:
    void RequestDestination(AIContext& context) {
        if (const Vector3* position = context.blackboard.TryGet(SensedEnemyKeys::kTargetPosition)) {
            RequireCommand(context.commands.RequestMoveTo(*position));
        } else {
            RequireCommand(context.commands.RequestStopMovement());
        }
    }
};

/// <summary>
/// Blackboardの対象IDへ一度だけ攻撃を要求する状態です。
/// </summary>
class SensedAttackState final : public IAgentState {
public:
    void Enter(AIContext& context) override {
        RequireCommand(context.commands.RequestStopMovement());
        if (const GameObjectId* targetId = context.blackboard.TryGet(SensedEnemyKeys::kTargetId);
            targetId && *targetId != kInvalidGameObjectId) {
            RequireCommand(context.commands.RequestAttack(*targetId));
        }
    }
    void Tick([[maybe_unused]] AIContext& context) override {}
    void Exit([[maybe_unused]] AIContext& context) override {}
};

/// 距離だけで発見を判定する例です。遮蔽・視野角などはゲーム側のSensorで差し替えてください。
inline void ConfigureDistanceSensor(
    SensorComponent& sensor, GameObjectId targetId,
    float detectionRadius, float attackRange) {
    if (targetId == kInvalidGameObjectId || !std::isfinite(detectionRadius) ||
        !std::isfinite(attackRange) || detectionRadius < 0.0f ||
        attackRange < 0.0f || attackRange > detectionRadius) {
        throw std::invalid_argument("Enemy sensor requires a valid target and ranges.");
    }
    sensor.AddSensor([targetId, detectionRadius, attackRange](
        const ComponentUpdateContext& update,
        const GameObject& owner, Blackboard& blackboard) {
        const auto clearTarget = [&blackboard] {
            if (!blackboard.Set(SensedEnemyKeys::kVisible, false) ||
                !blackboard.Set(SensedEnemyKeys::kInAttackRange, false)) {
                throw std::logic_error("Enemy sensor Blackboard key has the wrong type.");
            }
            blackboard.Remove(SensedEnemyKeys::kTargetId.name);
            blackboard.Remove(SensedEnemyKeys::kTargetPosition.name);
            blackboard.Remove(SensedEnemyKeys::kTargetDistance.name);
        };
        const GameObject* target = update.world ? update.world->FindObject(targetId) : nullptr;
        if (!target || !target->IsActive() || target->IsDestroyRequested()) {
            clearTarget();
            return;
        }

        const Vector3 position = target->GetTransform().GetWorldPosition();
        const Vector3 origin = owner.GetTransform().GetWorldPosition();
        const float distance = static_cast<float>(std::hypot(
            static_cast<double>(position.x) - origin.x,
            static_cast<double>(position.y) - origin.y,
            static_cast<double>(position.z) - origin.z));
        if (!std::isfinite(distance) || distance > detectionRadius) {
            clearTarget();
            return;
        }
        if (!blackboard.Set(SensedEnemyKeys::kVisible, true) ||
            !blackboard.Set(SensedEnemyKeys::kInAttackRange, distance <= attackRange) ||
            !blackboard.Set(SensedEnemyKeys::kTargetId, targetId) ||
            !blackboard.Set(SensedEnemyKeys::kTargetPosition, position) ||
            !blackboard.Set(SensedEnemyKeys::kTargetDistance, distance)) {
            throw std::logic_error("Enemy sensor Blackboard key has the wrong type.");
        }
    });
}

/// Sensor → Blackboard → FSM → CommandBuffer用の最小Enemy Brainです。
inline std::unique_ptr<StateMachineBrain> BuildSensedEnemyBrain() {
    auto brain = std::make_unique<StateMachineBrain>();
    brain->AddState("Idle", std::make_unique<SensedIdleState>());
    brain->AddState("Chase", std::make_unique<SensedChaseState>());
    brain->AddState("Attack", std::make_unique<SensedAttackState>());
    brain->SetInitialState("Idle");
    brain->AddTransition("Idle", "Chase", [](const AIContext& context) {
        return context.blackboard.GetOr(SensedEnemyKeys::kVisible, false);
    });
    brain->AddTransition("Chase", "Idle", [](const AIContext& context) {
        return !context.blackboard.GetOr(SensedEnemyKeys::kVisible, false);
    }, 10);
    brain->AddTransition("Chase", "Attack", [](const AIContext& context) {
        return context.blackboard.GetOr(SensedEnemyKeys::kInAttackRange, false);
    });
    brain->AddTransition("Attack", "Idle", [](const AIContext& context) {
        return !context.blackboard.GetOr(SensedEnemyKeys::kVisible, false);
    }, 10);
    brain->AddTransition("Attack", "Chase", [](const AIContext& context) {
        return !context.blackboard.GetOr(SensedEnemyKeys::kInAttackRange, false);
    });
    return brain;
}

} // namespace TakeC::Examples
