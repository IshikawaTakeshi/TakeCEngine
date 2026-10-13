#include "engine/GameObject/NavigationAgentComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/CharacterMotorComponent.h"
#include "engine/GameObject/EngineServices.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Navigation/IPathfindingService.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace TakeC {

void NavigationAgentComponent::OnCommandAttach() {
    if (GetOwner().GetComponent<AIMovementComponent>()) {
        throw std::logic_error("Use NavigationAgent or AIMovement, not both on one GameObject.");
    }
}

void NavigationAgentComponent::SetRepathDistance(float distance) {
    if (!std::isfinite(distance) || distance < 0.0f) {
        throw std::invalid_argument("Navigation repath distance must be finite and non-negative.");
    }
    repathDistance_ = distance;
}

void NavigationAgentComponent::SetWaypointTolerance(float distance) {
    if (!std::isfinite(distance) || distance < 0.0f) {
        throw std::invalid_argument("Navigation waypoint tolerance must be finite and non-negative.");
    }
    waypointTolerance_ = distance;
}

void NavigationAgentComponent::RequestRepath() {
    if (goal_) {
        needsRoute_ = true;
        waypoints_.clear();
        waypointIndex_ = 0;
    }
}

bool NavigationAgentComponent::HandleCommand(const AICommand& command) {
    if (const auto* move = std::get_if<AIMoveToCommand>(&command)) {
        if (!std::isfinite(move->targetPosition.x) || !std::isfinite(move->targetPosition.y) ||
            !std::isfinite(move->targetPosition.z) || !std::isfinite(move->stoppingDistance) ||
            move->stoppingDistance < 0.0f) {
            throw std::invalid_argument("Navigation destination must be finite and stopping distance non-negative.");
        }
        const Vector3 previous = goal_ ? goal_->targetPosition : move->targetPosition;
        const double change = std::hypot(
            static_cast<double>(move->targetPosition.x) - previous.x,
            static_cast<double>(move->targetPosition.y) - previous.y,
            static_cast<double>(move->targetPosition.z) - previous.z);
        if (!goal_ || change > repathDistance_ ||
            goal_->stoppingDistance != move->stoppingDistance ||
            (status_ == NavigationStatus::Arrived && change > 0.0)) {
            goal_ = *move;
            RequestRepath();
        }
        return true;
    }
    if (std::holds_alternative<AIStopMovementCommand>(command)) {
        CancelExecution();
        return true;
    }
    return false;
}

void NavigationAgentComponent::BuildRoute(const ComponentUpdateContext& context) {
    if (!goal_) {
        return;
    }
    const Vector3 start = GetOwner().GetTransform().GetWorldPosition();
    std::optional<std::vector<Vector3>> path;
    IPathfindingService* service = context.services ?
        context.services->TryGet<IPathfindingService>() : nullptr;
    if (service) {
        path = service->FindPath(start, goal_->targetPosition);
    } else {
        path = std::vector<Vector3>{goal_->targetPosition};
    }
    needsRoute_ = false;
    if (!path) {
        waypoints_.clear();
        waypointIndex_ = 0;
        status_ = NavigationStatus::NoPath;
        return;
    }
    if (path->empty() || path->back() != goal_->targetPosition) {
        throw std::logic_error("Pathfinding service must return a non-empty path ending at the goal.");
    }
    for (const Vector3& waypoint : *path) {
        if (!std::isfinite(waypoint.x) || !std::isfinite(waypoint.y) ||
            !std::isfinite(waypoint.z)) {
            throw std::logic_error("Pathfinding service returned a non-finite waypoint.");
        }
    }
    waypoints_ = std::move(*path);
    waypointIndex_ = 0;
    status_ = NavigationStatus::Following;
}

void NavigationAgentComponent::UpdateExecution(const ComponentUpdateContext& context) {
    if (!goal_) {
        return;
    }
    CharacterMotorComponent* motor = GetOwner().GetComponent<CharacterMotorComponent>();
    if (!motor) {
        throw std::logic_error("NavigationAgent requires a CharacterMotorComponent.");
    }
    if (!motor->IsEnabled()) {
        return;
    }
    if (needsRoute_) {
        BuildRoute(context);
    }
    if (status_ != NavigationStatus::Following) {
        return;
    }
    while (waypointIndex_ < waypoints_.size()) {
        const bool last = waypointIndex_ + 1 == waypoints_.size();
        const float tolerance = last ? goal_->stoppingDistance : waypointTolerance_;
        const Vector3 before = GetOwner().GetTransform().GetWorldPosition();
        const bool reached = motor->MoveToward(waypoints_[waypointIndex_], tolerance, context.deltaTime);
        if (!reached) {
            break;
        }
        ++waypointIndex_;
        if (before != GetOwner().GetTransform().GetWorldPosition()) {
            break; // 次の区間に同じdeltaTimeを再利用しません。
        }
    }
    if (waypointIndex_ == waypoints_.size()) {
        status_ = NavigationStatus::Arrived;
    }
}

void NavigationAgentComponent::CancelExecution() {
    goal_.reset();
    waypoints_.clear();
    waypointIndex_ = 0;
    needsRoute_ = false;
    status_ = NavigationStatus::Idle;
}

} // namespace TakeC
