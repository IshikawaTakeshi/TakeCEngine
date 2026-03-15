#include "SelectorNode.h"
#include "engine/Base/ImGuiManager.h"

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus SelectorNode::Execute(Blackboard& blackboard) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = children_[currentIndex_]->Execute(blackboard);

		if (status == BehaviorStatus::Running) {
			return BehaviorStatus::Running;
		}
		if (status == BehaviorStatus::Success) {
			Reset();
			return BehaviorStatus::Success;
		}
		currentIndex_++;
	}
	Reset();
	return BehaviorStatus::Failure;
}