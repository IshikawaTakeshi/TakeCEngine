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
BehaviorStatus ScoreConditionNode::Execute(PlayableCharacterInfo&) {
	float score = scoreFunc_();
	if (score >= threshold_) {
		return BehaviorStatus::Success;
	}
	return BehaviorStatus::Failure;
}
