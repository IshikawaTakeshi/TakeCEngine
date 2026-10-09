#include "engine/GameObject/AIControllerComponent.h"

#include <cmath>
#include <stdexcept>
#include <utility>

using namespace TakeC;

AIControllerComponent::ExecutionGuard::ExecutionGuard(AIControllerComponent& controller)
    : controller_(controller) {
    controller_.EnsureNotExecuting();
    controller_.executing_ = true;
}

AIControllerComponent::ExecutionGuard::~ExecutionGuard() {
    controller_.executing_ = false;
}

void AIControllerComponent::EnsureNotExecuting() const {
    if (executing_) {
        throw std::logic_error("Cannot modify an AIController during Start, Tick or Reset.");
    }
}

void AIControllerComponent::SetBrain(std::unique_ptr<IAgentBrain> brain) {
    EnsureNotExecuting();
    Reset();
    brain_ = std::move(brain);
    brainStarted_ = false;
    playing_ = brain_ != nullptr;
}

void AIControllerComponent::SetTickInterval(float seconds) {
    EnsureNotExecuting();
    if (!std::isfinite(seconds) || seconds < 0.0f) {
        throw std::invalid_argument("AI tick interval must be finite and non-negative.");
    }
    tickInterval_ = seconds;
    ResetTickClock();
}

void AIControllerComponent::Reset() {
    EnsureNotExecuting();
    ExecutionGuard guard(*this);
    if (brain_ && attachedOwner_) {
        AIContext context{*attachedOwner_, blackboard_, 0.0f};
        brain_->Reset(context);
    }
    ResetTickClock();
}

void AIControllerComponent::Stop() {
    Reset();
    playing_ = false;
}

void AIControllerComponent::Restart() {
    Reset();
    Resume();
}

void AIControllerComponent::OnAttach() {
    attachedOwner_ = &GetOwner();
}

void AIControllerComponent::Update(const ComponentUpdateContext& context) {
    if (!brain_ || !playing_) {
        return;
    }

    
    EnsureNotExecuting();
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f ||
        !std::isfinite(elapsedTime_ + context.deltaTime)) {
        throw std::invalid_argument("AI deltaTime must be finite and non-negative.");
    }
    ExecutionGuard guard(*this);
    if (!brainStarted_) {
        AIContext startContext{GetOwner(), blackboard_, 0.0f};
        brain_->Start(startContext);
        brainStarted_ = true;
        if (!playing_) {
            return;
        }
    }
    elapsedTime_ += context.deltaTime;
    if (elapsedTime_ < tickInterval_) {
        return;
    }
    AIContext execution{GetOwner(), blackboard_, elapsedTime_};
    ResetTickClock();
    brain_->Tick(execution);
    if (brain_->IsFinished()) {
        playing_ = false;
    }
}

void AIControllerComponent::OnDetach() {
    Stop();
    attachedOwner_ = nullptr;
    brainStarted_ = false;
}
