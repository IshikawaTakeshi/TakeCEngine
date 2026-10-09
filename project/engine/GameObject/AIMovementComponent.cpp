#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Math/MatrixMath.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace TakeC {

void AIMovementComponent::SetSpeed(float unitsPerSecond) {
    if (!std::isfinite(unitsPerSecond) || unitsPerSecond < 0.0f) {
        throw std::invalid_argument("AI movement speed must be finite and non-negative.");
    }
    speed_ = unitsPerSecond;
}

bool AIMovementComponent::HandleCommand(const AICommand& command) {
    if (const auto* move = std::get_if<AIMoveToCommand>(&command)) {
        targetPosition_ = move->targetPosition;
        stoppingDistance_ = move->stoppingDistance;
        return true;
    }
    if (std::holds_alternative<AIStopMovementCommand>(command)) {
        CancelExecution();
        return true;
    }
    return false;
}

void AIMovementComponent::UpdateExecution(const ComponentUpdateContext& context) {
    if (!targetPosition_) {
        return;
    }
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f) {
        throw std::invalid_argument("AI movement deltaTime must be finite and non-negative.");
    }
    auto& transform = GetOwner().GetTransform();
    const Vector3 position = transform.GetWorldPosition();
    const double dx = static_cast<double>(targetPosition_->x) - position.x;
    const double dy = static_cast<double>(targetPosition_->y) - position.y;
    const double dz = static_cast<double>(targetPosition_->z) - position.z;
    const double distance = std::hypot(dx, dy, dz);
    if (!std::isfinite(distance)) {
        throw std::runtime_error("AI movement requires a finite world transform.");
    }
    if (distance <= stoppingDistance_) {
        CancelExecution();
        return;
    }
    const double step = static_cast<double>(speed_) * context.deltaTime;
    const double ratio = std::min(step / distance, 1.0);
    const Vector3 nextWorld{
        static_cast<float>(position.x + dx * ratio),
        static_cast<float>(position.y + dy * ratio),
        static_cast<float>(position.z + dz * ratio)};
    Vector3 nextLocal = nextWorld;
    if (const auto* parent = transform.GetParent()) {
        const Matrix4x4 inverse = MatrixMath::Inverse(parent->GetWorldMatrix());
        for (const auto& row : inverse.m) {
            for (const float element : row) {
                if (!std::isfinite(element)) {
                    throw std::runtime_error("AI movement requires an invertible parent transform.");
                }
            }
        }
        nextLocal = MatrixMath::Transform(nextWorld, inverse);
    }
    if (!std::isfinite(nextLocal.x) || !std::isfinite(nextLocal.y) || !std::isfinite(nextLocal.z)) {
        throw std::runtime_error("AI movement produced a non-finite local position.");
    }
    transform.SetLocalPosition(nextLocal);
    if (step >= distance) {
        CancelExecution();
    }
}

} // namespace TakeC
