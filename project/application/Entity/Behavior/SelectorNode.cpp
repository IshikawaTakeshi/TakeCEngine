#include "SelectorNode.h"

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus SelectorNode::Execute(PlayableCharacterInfo& characterInfo) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = children_[currentIndex_]->Execute(characterInfo);

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