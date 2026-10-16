#include "engine/GameObject/AIControllerComponent.h"
#include "engine/GameObject/AICommandHandlerComponent.h"
#include "engine/GameObject/GameObject.h"

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
        AIContext context{*attachedOwner_, blackboard_, commands_, 0.0f,
            attachedOwner_->GetWorld(), attachedOwner_->GetServices()};
        brain_->Reset(context);
    }
    ResetCommandExecution();
    ResetTickClock();
}

void AIControllerComponent::ResetCommandExecution() {
    commands_.Clear();
    lastUnhandledCommandCount_ = 0;
    if (attachedOwner_) {
        for (auto* handler : attachedOwner_->GetComponents<AICommandHandlerComponent>()) {
            if (handler->UsesController(*this)) {
                handler->CancelExecution();
            }
        }
    }
    // CancelExecutionが追加した要求も破棄します。
    commands_.Clear();
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
    try {
        if (!brainStarted_) {
            AIContext startContext{GetOwner(), blackboard_, commands_, 0.0f,
                context.world, context.services};
            brain_->Initialize(startContext);
            brainStarted_ = true;
            if (!playing_) {
                return;
            }
        }
        elapsedTime_ += context.deltaTime;
        if (elapsedTime_ < tickInterval_) {
            return;
        }
        AIContext execution{GetOwner(), blackboard_, commands_, elapsedTime_,
            context.world, context.services};
        ResetTickClock();
        brain_->Tick(execution);
        if (brain_->IsFinished()) {
            playing_ = false;
        }
    } catch (...) {
        ResetCommandExecution();
        throw;
    }
}

void AIControllerComponent::LateUpdate(const ComponentUpdateContext& context) {
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f) {
        throw std::invalid_argument("AI execution deltaTime must be finite and non-negative.");
    }
    ExecutionGuard guard(*this);
    const auto handlers = GetOwner().GetComponents<AICommandHandlerComponent>();
    // スナップショットを消費するため、Handler内で追加した要求は次回へ送られます。
    const auto batch = commands_.ConsumeAll();
    lastUnhandledCommandCount_ = 0;
    try {
        for (const auto& command : batch) {
            if (GetOwner().IsDestroyRequested() || !GetOwner().IsActive()) {
                ResetCommandExecution();
                return;
            }
            bool handled = false;
            for (auto* handler : handlers) {
                if (handler->IsEnabled() && handler->UsesController(*this) &&
                    handler->HandleCommand(command)) {
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                ++lastUnhandledCommandCount_;
            }
        }
        for (auto* handler : handlers) {
            if (GetOwner().IsDestroyRequested() || !GetOwner().IsActive()) {
                ResetCommandExecution();
                return;
            }
            if (handler->IsEnabled() && handler->UsesController(*this)) {
                handler->UpdateExecution(context);
            }
        }
    } catch (...) {
        ResetCommandExecution();
        throw;
    }
}

void AIControllerComponent::OnDetach() {
    Stop();
    attachedOwner_ = nullptr;
    brainStarted_ = false;
}
