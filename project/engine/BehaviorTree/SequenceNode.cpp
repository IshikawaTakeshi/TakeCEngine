#include "SequenceNode.h"

//==================================================================================
// ノードの実行 
//==================================================================================

BehaviorStatus SequenceNode::Execute(Blackboard& blackboard) {
	return ExecuteImpl(blackboard, nullptr);
}

BehaviorStatus SequenceNode::Tick(const TakeC::BehaviorTreeContext& context) {
	return ExecuteImpl(context.blackboard, &context);
}

BehaviorStatus SequenceNode::ExecuteImpl(
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
