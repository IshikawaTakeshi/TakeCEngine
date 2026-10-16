#include "engine/BehaviorTree/BehaviorTreeBrain.h"
#include "engine/BehaviorTree/CompositeNode.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace TakeC {
namespace {
class ExecutionGuard final {
public:
    explicit ExecutionGuard(bool& flag) : flag_(flag) { flag_ = true; }
    ~ExecutionGuard() { flag_ = false; }
private:
    bool& flag_;
};
} // namespace

void BehaviorTreeBrain::EnsureNotExecuting() const {
    if (executing_) {
        throw std::logic_error("Cannot modify a BehaviorTree during Tick or Reset.");
    }
}

void BehaviorTreeBrain::SetTree(std::unique_ptr<BehaviorNode> root) {
    EnsureNotExecuting();
    ExecutionGuard guard(executing_);
    ResetTree();
    root_ = std::move(root);
    if (root_) {
        BindNodes(*root_);
    }
}

void BehaviorTreeBrain::BindAction(
    std::string actionId, ActionNode::ContextExecuteCallback execute,
    ActionNode::ResetCallback reset) {
    EnsureNotExecuting();
    if (actionId.empty() || !execute) {
        throw std::invalid_argument("BehaviorTree action requires an ID and callback.");
    }
    ExecutionGuard guard(executing_);
    ResetTree();
    actions_.insert_or_assign(std::move(actionId), ActionBinding{std::move(execute), std::move(reset)});
    if (root_) {
        BindNodes(*root_);
    }
}

void BehaviorTreeBrain::BindNodes(BehaviorNode& node) {
    if (auto* action = dynamic_cast<ActionNode*>(&node)) {
        if (const auto it = actions_.find(action->GetActionId()); it != actions_.end()) {
            action->SetContextExecuteCallback(it->second.execute);
            action->SetResetCallback(it->second.reset);
        }
    }
    if (auto* composite = dynamic_cast<CompositeNode*>(&node)) {
        for (const auto& child : composite->GetChildren()) {
            BindNodes(*child);
        }
    }
}

void BehaviorTreeBrain::ResetTree() {
    if (root_) {
        root_->Reset();
    }
    status_ = BehaviorStatus::Invalid;
    restartPending_ = false;
    finished_ = false;
}

void BehaviorTreeBrain::Reset() {
    EnsureNotExecuting();
    ExecutionGuard guard(executing_);
    ResetTree();
}

void BehaviorTreeBrain::Tick(AIContext& context) {
    EnsureNotExecuting();
    if (!std::isfinite(context.deltaTime) || context.deltaTime < 0.0f) {
        throw std::invalid_argument("BehaviorTree deltaTime must be finite and non-negative.");
    }
    ExecutionGuard guard(executing_);
    if (!root_) {
        finished_ = true;
        return;
    }
    if (restartPending_) {
        ResetTree();
    }
    status_ = root_->Tick(context);
    switch (status_) {
    case BehaviorStatus::Success:
    case BehaviorStatus::Failure:
        restartPending_ = true;
        finished_ = !repeat_;
        break;
    case BehaviorStatus::Running:
        finished_ = false;
        break;
    default:
        finished_ = true;
        restartPending_ = true;
        break;
    }
}
} // namespace TakeC
