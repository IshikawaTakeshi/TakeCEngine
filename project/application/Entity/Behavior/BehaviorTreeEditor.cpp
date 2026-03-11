#include "BehaviorTreeEditor.h"

//==================================================================================
// 初期化
//==================================================================================
void BehaviorTreeEditor::Initialize() {

	// ImNodeFlowエディタの初期化
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");

	//右クリックメニューのノードを追加可能にする
	flowEditor_->rightClickPopUpContent([this](ImFlow::BaseNode* node) {
		if (ImGui::MenuItem("Add Action")) { flowEditor_->placeNode<ActionNodeView>("NONE"); }
		if (ImGui::MenuItem("Add Condition")) { flowEditor_->placeNode<ConditionNodeView>(); }
		if (ImGui::MenuItem("Add ScoreCondition")) { flowEditor_->placeNode<ScoreConditionNodeView>(); }
		if (ImGui::MenuItem("Add Selector")) { flowEditor_->placeNode<SelectorNodeView>(); }
		if (ImGui::MenuItem("Add Sequence")) { flowEditor_->placeNode<SequenceNodeView>(); }
		if (ImGui::MenuItem("Add PlannerSelector")) { flowEditor_->placeNode<PlannerSelectorNodeView>(); }
		if (ImGui::MenuItem("Add WeightSelector")) { flowEditor_->placeNode<WeightSelectorNodeView>(); }

		});
}

//===============================================================================
// 描画
//===============================================================================
void BehaviorTreeEditor::Draw() {
	if (flowEditor_) {
		flowEditor_->update();
	}
}
