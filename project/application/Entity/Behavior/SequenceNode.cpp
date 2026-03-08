#include "SequenceNode.h"

//==================================================================================
// ノードの実行 
//==================================================================================

BehaviorStatus SequenceNode::Execute(PlayableCharacterInfo& characterInfo) {
	while (currentIndex_ < children_.size()) {
		BehaviorStatus status = children_[currentIndex_]->Execute(characterInfo);

		if (status == BehaviorStatus::Running) {
			return BehaviorStatus::Running; // まだ実行中、次フレームで継続
		}
		if (status == BehaviorStatus::Failure) {
			Reset(); // 失敗したらシーケンス全体を失敗にする
			return BehaviorStatus::Failure;
		}
		// SUCCESS → 次の子ノードへ
		currentIndex_++;
	}

	// 全ての子ノードが成功
	Reset();
	return BehaviorStatus::Success;
}