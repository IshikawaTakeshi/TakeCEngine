#include "engine/GameObject/BehaviorTreeComponent.h"

#include <stdexcept>
#include <utility>

using namespace TakeC;
BehaviorTreeComponent::BehaviorTreeComponent() {
    SetBrain(std::make_unique<BehaviorTreeBrain>());
}

void BehaviorTreeComponent::SetBrain(std::unique_ptr<IAgentBrain> brain) {
    EnsureNotExecuting();
    if (!dynamic_cast<BehaviorTreeBrain*>(brain.get())) {
        throw std::invalid_argument("BehaviorTreeComponent requires a non-null BehaviorTreeBrain.");
    }
    AIControllerComponent::SetBrain(std::move(brain));
    if (!GetTree()) {
        Pause();
    }
}

BehaviorTreeBrain& BehaviorTreeComponent::GetBehaviorTreeBrain() {
    return static_cast<BehaviorTreeBrain&>(*GetBrain());
}

const BehaviorTreeBrain& BehaviorTreeComponent::GetBehaviorTreeBrain() const {
    return static_cast<const BehaviorTreeBrain&>(*GetBrain());
}

void BehaviorTreeComponent::SetTree(std::unique_ptr<BehaviorNode> root) {
    ExecutionGuard guard(*this);
    GetBehaviorTreeBrain().SetTree(std::move(root));
    ResetTickClock();
    if (GetTree()) {
        Resume();
    } else {
        Pause();
    }
}

void BehaviorTreeComponent::BindAction(
    std::string actionId, ActionNode::ContextExecuteCallback execute,
    ActionNode::ResetCallback reset) {
    ExecutionGuard guard(*this);
    GetBehaviorTreeBrain().BindAction(std::move(actionId), std::move(execute), std::move(reset));
    ResetTickClock();
}

void BehaviorTreeComponent::Resume() {
    if (GetTree()) {
        AIControllerComponent::Resume();
    }
}
