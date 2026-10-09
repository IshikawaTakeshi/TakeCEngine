#pragma once

#include "engine/AI/IAgentBrain.h"
#include "engine/BehaviorTree/ActionNode.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace TakeC {
/// ツリー固有の状態を所有するBrain。更新・Blackboardの所有はControllerに任せます。
class BehaviorTreeBrain final : public IAgentBrain {
public:
    void Tick(AIContext& context) override;
    void Reset([[maybe_unused]] AIContext& context) override { Reset(); }
    bool IsFinished() const override { return finished_; }

    void SetTree(std::unique_ptr<BehaviorNode> root);
    BehaviorNode* GetTree() const { return root_.get(); }
    void BindAction(
        std::string actionId, ActionNode::ContextExecuteCallback execute,
        ActionNode::ResetCallback reset = {});
    void SetRepeat(bool repeat) { repeat_ = repeat; }
    bool IsRepeat() const { return repeat_; }
    BehaviorStatus GetStatus() const { return status_; }
    void Reset();

private:
    struct ActionBinding {
        ActionNode::ContextExecuteCallback execute;
        ActionNode::ResetCallback reset;
    };
    void BindNodes(BehaviorNode& node);
    void ResetTree();
    void EnsureNotExecuting() const;

    std::unique_ptr<BehaviorNode> root_;
    std::unordered_map<std::string, ActionBinding> actions_;
    BehaviorStatus status_ = BehaviorStatus::Invalid;
    bool repeat_ = true;
    bool restartPending_ = false;
    bool finished_ = false;
    bool executing_ = false;
};
} // namespace TakeC
