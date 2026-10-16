#include "engine/GameObject/CharacterMotorComponent.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Math/MatrixMath.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace TakeC {

void CharacterMotorComponent::SetSpeed(float unitsPerSecond) {
    if (!std::isfinite(unitsPerSecond) || unitsPerSecond < 0.0f) {
        throw std::invalid_argument("Character motor speed must be finite and non-negative.");
    }
    speed_ = unitsPerSecond;
}

bool CharacterMotorComponent::MoveToward(
    const Vector3& worldTarget, float stoppingDistance, float deltaTime) {
    return StepTransform(GetOwner().GetTransform(), worldTarget, speed_, stoppingDistance, deltaTime);
}

bool CharacterMotorComponent::StepTransform(
    TransformComponent& transform, const Vector3& worldTarget,
    float speed, float stoppingDistance, float deltaTime) {
    if (!std::isfinite(worldTarget.x) || !std::isfinite(worldTarget.y) ||
        !std::isfinite(worldTarget.z) || !std::isfinite(speed) || speed < 0.0f ||
        !std::isfinite(stoppingDistance) || stoppingDistance < 0.0f ||
        !std::isfinite(deltaTime) || deltaTime < 0.0f) {
        throw std::invalid_argument("Character motor inputs must be finite and non-negative where required.");
    }

    const Vector3 position = transform.GetWorldPosition();
    const double dx = static_cast<double>(worldTarget.x) - position.x;
    const double dy = static_cast<double>(worldTarget.y) - position.y;
    const double dz = static_cast<double>(worldTarget.z) - position.z;
    const double distance = std::hypot(dx, dy, dz);
    if (!std::isfinite(distance)) {
        throw std::runtime_error("Character motor requires a finite world transform.");
    }
    if (distance <= stoppingDistance) {
        return true;
    }
    const double step = static_cast<double>(speed) * deltaTime;
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
                    throw std::runtime_error("Character motor requires an invertible parent transform.");
                }
            }
        }
        nextLocal = MatrixMath::Transform(nextWorld, inverse);
    }
    if (!std::isfinite(nextLocal.x) || !std::isfinite(nextLocal.y) || !std::isfinite(nextLocal.z)) {
        throw std::runtime_error("Character motor produced a non-finite local position.");
    }
    transform.SetLocalPosition(nextLocal);
    return step >= distance;
}

} // namespace TakeC
