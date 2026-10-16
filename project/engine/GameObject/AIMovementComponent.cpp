#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/CharacterMotorComponent.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/NavigationAgentComponent.h"
#include "engine/GameObject/TransformComponent.h"

#include <cmath>
#include <stdexcept>

namespace TakeC {

void AIMovementComponent::OnCommandAttach() {
    if (GetOwner().GetComponent<NavigationAgentComponent>()) {
        throw std::logic_error("Use AIMovement or NavigationAgent, not both on one GameObject.");
    }
}

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
    if (CharacterMotorComponent::StepTransform(
        GetOwner().GetTransform(), *targetPosition_, speed_, stoppingDistance_, context.deltaTime)) {
        CancelExecution();
    }
}

} // namespace TakeC
