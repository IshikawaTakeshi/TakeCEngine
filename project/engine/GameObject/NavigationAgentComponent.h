#pragma once

#include "engine/GameObject/AICommandHandlerComponent.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace TakeC {

/// <summary>NavigationAgentの移動要求の実行状態です。</summary>
enum class NavigationStatus { Idle, Following, Arrived, NoPath };

/// <summary>
/// MoveToを経路要求へ変換し、得られたウェイポイントをCharacterMotorへ渡します。
/// 経路探索サービスがない場合は終点への直線経路を使用します。
/// </summary>
class NavigationAgentComponent final : public AICommandHandlerComponent {
public:
    bool HandleCommand(const AICommand& command) override;
    void UpdateExecution(const ComponentUpdateContext& context) override;
    void CancelExecution() override;

    /// この距離を超えて目的地が変わった場合に同期的に再探索します。
    void SetRepathDistance(float distance);
    float GetRepathDistance() const noexcept { return repathDistance_; }
    void SetWaypointTolerance(float distance);
    float GetWaypointTolerance() const noexcept { return waypointTolerance_; }
    void RequestRepath();

    NavigationStatus GetStatus() const noexcept { return status_; }
    bool IsMoving() const noexcept { return status_ == NavigationStatus::Following || needsRoute_; }
    const std::optional<AIMoveToCommand>& GetGoal() const noexcept { return goal_; }
    const std::vector<Vector3>& GetWaypoints() const noexcept { return waypoints_; }
    std::size_t GetCurrentWaypointIndex() const noexcept { return waypointIndex_; }

protected:
    void OnCommandAttach() override;

private:
    void BuildRoute(const ComponentUpdateContext& context);

    std::optional<AIMoveToCommand> goal_;
    std::vector<Vector3> waypoints_;
    std::size_t waypointIndex_ = 0;
    float repathDistance_ = 0.5f;
    float waypointTolerance_ = 0.05f;
    NavigationStatus status_ = NavigationStatus::Idle;
    bool needsRoute_ = false;
};

} // namespace TakeC
