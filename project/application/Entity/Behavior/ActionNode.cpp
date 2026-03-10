#include "ActionNode.h"

//==================================================================================
// コンストラクタ
//==================================================================================
ActionNode::ActionNode(
	GameCharacterState targetState,
	GameCharacterStateManager* stateManager,
	const std::string& name) {

	targetState_ = targetState;
	stateManager_ = stateManager;
	name_ = name;
}

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus ActionNode::Execute(Blackboard&) {
	if (!isStarted_) {
		//ステート遷移をリクエスト
		stateManager_->RequestState(targetState_);
		isStarted_ = true;
	}
	//現在のステートがターゲットと一致した場合、まだ実行中
	if (stateManager_->GetCurrentStateType() == targetState_) {
		return BehaviorStatus::Running;
	}

	//ステートが変わった場合、アクション完了とみなす
	if (isStarted_ && stateManager_->GetCurrentStateType() != targetState_) {
		Reset(); // 次回の実行に備えて状態をリセット
		return BehaviorStatus::Success;
	}

	return BehaviorStatus::Running; // まだ実行中
}

void ActionNode::Reset() {
	isStarted_ = false;
}
