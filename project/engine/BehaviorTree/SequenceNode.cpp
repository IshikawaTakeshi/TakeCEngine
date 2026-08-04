#include "SequenceNode.h"

//==================================================================================
// ノードの実行 
//==================================================================================

BehaviorStatus SequenceNode::Execute(Blackboard& blackboard) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = children_[currentIndex_]->Execute(blackboard);

		if (status == BehaviorStatus::Running) {
			currentStatus_ = BehaviorStatus::Running;
			return BehaviorStatus::Running; // まだ実行中、次フレームで継続
		}
		if (status == BehaviorStatus::Failure) {
			Reset(); // 失敗したらシーケンス全体を失敗にする
			currentStatus_ = BehaviorStatus::Failure;
			return BehaviorStatus::Failure;
		}
		// SUCCESS → 次の子ノードへ
		currentIndex_++;
	}

	// 全ての子ノードが成功
	Reset();
	currentStatus_ = BehaviorStatus::Success;
	return BehaviorStatus::Success;
}