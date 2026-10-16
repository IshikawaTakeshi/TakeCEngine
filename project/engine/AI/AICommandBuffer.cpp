#include "engine/AI/AICommandBuffer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace TakeC {
namespace {
bool IsMovement(const AICommand& command) {
    return std::holds_alternative<AIMoveToCommand>(command) ||
        std::holds_alternative<AIStopMovementCommand>(command);
}
} // namespace

AICommandBuffer::AICommandBuffer(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) {
        throw std::invalid_argument("AICommandBuffer capacity must be positive.");
    }
    commands_.reserve(capacity_);
}

bool AICommandBuffer::RequestMovement(AICommand command) {
    const bool replacesMovement = std::ranges::any_of(commands_, IsMovement);
    if (!replacesMovement && commands_.size() >= capacity_) {
        return false;
    }
    std::erase_if(commands_, IsMovement);
    commands_.push_back(std::move(command));
    return true;
}

bool AICommandBuffer::RequestMoveTo(const Vector3& worldPosition, float stoppingDistance) {
    if (!std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) ||
        !std::isfinite(worldPosition.z) || !std::isfinite(stoppingDistance) ||
        stoppingDistance < 0.0f) {
        throw std::invalid_argument("AI move target and stopping distance must be finite.");
    }
    return RequestMovement(AIMoveToCommand{worldPosition, stoppingDistance});
}

bool AICommandBuffer::RequestStopMovement() {
    return RequestMovement(AIStopMovementCommand{});
}

bool AICommandBuffer::RequestAttack(GameObjectId targetId, std::string actionId) {
    if (targetId == kInvalidGameObjectId || actionId.empty()) {
        throw std::invalid_argument("AI attack requires a valid target ID and action ID.");
    }
    if (commands_.size() >= capacity_) {
        return false;
    }
    commands_.push_back(AIAttackCommand{targetId, std::move(actionId)});
    return true;
}

std::vector<AICommand> AICommandBuffer::ConsumeAll() {
    std::vector<AICommand> result;
    result.reserve(commands_.size());
    for (AICommand& command : commands_) {
        result.push_back(std::move(command));
    }
    commands_.clear();
    return result;
}

} // namespace TakeC
