#include "engine/GameObject/AICommandHandlerComponent.h"
#include "engine/GameObject/AIControllerComponent.h"
#include "engine/GameObject/GameObject.h"

#include <stdexcept>

namespace TakeC {

void AICommandHandlerComponent::SetCommandSource(AIControllerComponent* source) {
    if (!commandOwner_) {
        throw std::logic_error("Attach the AI command handler before selecting its source.");
    }
    if (source && &source->GetOwner() != commandOwner_) {
        throw std::invalid_argument("AI command source must belong to the same GameObject.");
    }
    CancelExecution();
    source_ = source;
}

bool AICommandHandlerComponent::UsesController(const AIControllerComponent& controller) const {
    if (!commandOwner_) {
        return false;
    }
    const AIControllerComponent* source = source_ ? source_ :
        commandOwner_->GetComponent<AIControllerComponent>();
    return source == &controller;
}

void AICommandHandlerComponent::OnAttach() {
    commandOwner_ = &GetOwner();
    OnCommandAttach();
}

void AICommandHandlerComponent::OnDetach() {
    CancelExecution();
    OnCommandDetach();
    source_ = nullptr;
    commandOwner_ = nullptr;
}

} // namespace TakeC
