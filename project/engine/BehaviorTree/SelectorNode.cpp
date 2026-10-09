#include "SelectorNode.h"

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus SelectorNode::Execute(Blackboard& blackboard) {
	return ExecuteImpl(blackboard, nullptr);
}

BehaviorStatus SelectorNode::Tick(const TakeC::BehaviorTreeContext& context) {
	return ExecuteImpl(context.blackboard, &context);
}

BehaviorStatus SelectorNode::ExecuteImpl(
	Blackboard& blackboard, const TakeC::BehaviorTreeContext* context) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = ExecuteChild(*children_[currentIndex_], blackboard, context);

		if (status == BehaviorStatus::Aborted || status == BehaviorStatus::Invalid) {
			Reset();
			currentStatus_ = status;
			return status;
		}

		if (status == BehaviorStatus::Running) {
			currentStatus_ = BehaviorStatus::Running;
			return BehaviorStatus::Running;
		}
		if (status == BehaviorStatus::Success) {
			Reset();
			currentStatus_ = BehaviorStatus::Success;
			return BehaviorStatus::Success;
		}
		currentIndex_++;
	}
	Reset();
	currentStatus_ = BehaviorStatus::Failure;
	return BehaviorStatus::Failure;
}
