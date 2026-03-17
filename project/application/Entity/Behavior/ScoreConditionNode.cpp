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
	float score = scoreFunc_();
	if (score >= threshold_) {
		return BehaviorStatus::Success;
	}
	return BehaviorStatus::Failure;
}

//====================================================================================
// パラメータの編集
//====================================================================================
void ScoreConditionNode::DrawInspector() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::PushID(this);
	ImGui::DragFloat("Threshold", &threshold_, 0.01f, 0.0f, 1.0f);
	ImGui::PopID();
#endif
}
