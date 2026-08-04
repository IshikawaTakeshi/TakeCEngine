#include "SelectorNode.h"
#include "engine/Base/ImGuiManager.h"

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus SelectorNode::Execute(Blackboard& blackboard) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = children_[currentIndex_]->Execute(blackboard);

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