#include "ScoreConditionNode.h"

//====================================================================================
// コンストラクタ
//====================================================================================
ScoreConditionNode::ScoreConditionNode(ScoreFunc scoreFunc, float threshold, const std::string& name) {
	scoreFunc_ = std::move(scoreFunc);
	threshold_ = threshold;
	name_ = name;
}

//====================================================================================
// ノードの実行
//====================================================================================
BehaviorStatus ScoreConditionNode::Execute(Blackboard&) {
	if (!scoreFunc_) {
		currentStatus_ = BehaviorStatus::Failure;
		return currentStatus_;
	}

	const float score = scoreFunc_();
	if (score >= threshold_) {
		currentStatus_ = BehaviorStatus::Success;
		return currentStatus_;
	}
	currentStatus_ = BehaviorStatus::Failure;
	return currentStatus_;
}


