#include "engine/AI/StateMachineBrain.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace TakeC {

StateMachineBrain::ExecutionGuard::ExecutionGuard(StateMachineBrain& brain) : brain_(brain) {
    brain_.EnsureNotExecuting();
    brain_.executing_ = true;
}

StateMachineBrain::ExecutionGuard::~ExecutionGuard() {
    brain_.executing_ = false;
}

void StateMachineBrain::EnsureNotExecuting() const {
    if (executing_) {
        throw std::logic_error("Cannot modify or recursively execute a StateMachine during callbacks.");
    }
}

void StateMachineBrain::EnsureRegistered(std::string_view id) const {
    if (id.empty() || !states_.contains(std::string(id))) {
        throw std::invalid_argument("FSM state ID must refer to a registered state.");
    }
}

void StateMachineBrain::AddState(std::string id, std::unique_ptr<IAgentState> state) {
    EnsureNotExecuting();
    if (id.empty() || !state || states_.contains(id)) {
        throw std::invalid_argument("FSM state requires a unique non-empty ID and non-null instance.");
    }
    states_.emplace(std::move(id), std::move(state));
}

void StateMachineBrain::SetInitialState(std::string id) {
    EnsureNotExecuting();
    EnsureRegistered(id);
    initialStateId_ = std::move(id);
}

void StateMachineBrain::AddTransition(
    std::string from, std::string to, Condition condition, int priority) {
    EnsureNotExecuting();
    EnsureRegistered(from);
    EnsureRegistered(to);
    if (!condition || from == to) {
        throw std::invalid_argument("FSM transition requires a condition and different source/target.");
    }
    transitions_.push_back(Transition{std::move(from), std::move(to), std::move(condition), priority});
}

void StateMachineBrain::RequestTransition(std::string to, int priority) {
    if (evaluatingCondition_) {
        throw std::logic_error("FSM transition conditions must not request transitions.");
    }
    EnsureRegistered(to);
    if (!pendingRequest_ || priority >= pendingRequest_->priority) {
        pendingRequest_ = TransitionRequest{std::move(to), priority};
    }
}

void StateMachineBrain::EnterState(const std::string& id, AIContext& context) {
    currentStateId_ = id;
    currentState_ = states_.at(id).get();
    currentState_->Enter(context);
}

void StateMachineBrain::ExitCurrentState(AIContext& context) {
    // Exitが失敗しても同じStateを二重終了しないよう、先に実行状態を解除します。
    IAgentState* previous = std::exchange(currentState_, nullptr);
    if (previous) {
        try {
            previous->Exit(context);
        } catch (...) {
            currentStateId_.clear();
            throw;
        }
    }
    currentStateId_.clear();
}

void StateMachineBrain::EnsureActive(AIContext& context) {
    if (!currentState_) {
        if (initialStateId_.empty()) {
            throw std::logic_error("Configure an initial FSM state before starting.");
        }
        EnterState(initialStateId_, context);
    }
}

void StateMachineBrain::ResetRuntime(AIContext& context) {
    pendingRequest_.reset();
    try {
        ExitCurrentState(context);
    } catch (...) {
        pendingRequest_.reset();
        throw;
    }
    // Reset中のExitから予約された遷移も破棄します。
    pendingRequest_.reset();
}

bool StateMachineBrain::EvaluateCondition(const Condition& condition, const AIContext& context) {
    evaluatingCondition_ = true;
    try {
        const bool result = condition(context);
        evaluatingCondition_ = false;
        return result;
    } catch (...) {
        evaluatingCondition_ = false;
        throw;
    }
}

void StateMachineBrain::Initialize(AIContext& context) {
    ExecutionGuard guard(*this);
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f) {
        throw std::invalid_argument("FSM deltaTime must be finite and non-negative.");
    }
    try {
        EnsureActive(context);
    } catch (...) {
        ResetRuntime(context);
        throw;
    }
}

void StateMachineBrain::Tick(AIContext& context) {
    ExecutionGuard guard(*this);
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f) {
        throw std::invalid_argument("FSM deltaTime must be finite and non-negative.");
    }
    try {
        // このTickのEnter/Exit/Tickで発行する要求とは別のスナップショットです。
        auto selected = std::move(pendingRequest_);
        pendingRequest_.reset();
        EnsureActive(context);
        if (selected && selected->to == currentStateId_) {
            selected.reset(); // 明示的な同一Stateへの要求は再Enterしません。
        }
        for (const auto& transition : transitions_) {
            if (transition.from != currentStateId_ ||
                (selected && transition.priority <= selected->priority)) {
                continue;
            }
            if (EvaluateCondition(transition.condition, context)) {
                selected = TransitionRequest{transition.to, transition.priority};
            }
        }
        if (selected) {
            ExitCurrentState(context);
            EnterState(selected->to, context);
        }
        currentState_->Tick(context);
    } catch (...) {
        ResetRuntime(context);
        throw;
    }
}

void StateMachineBrain::Reset(AIContext& context) {
    ExecutionGuard guard(*this);
    ResetRuntime(context);
}

} // namespace TakeC
