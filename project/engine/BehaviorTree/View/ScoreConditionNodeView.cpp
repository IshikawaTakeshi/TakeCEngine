#include "ScoreConditionNodeView.h"
#include "engine/Base/ImGuiManager.h"

//====================================================================
// コンストラクタ
//====================================================================
ScoreConditionNodeView::ScoreConditionNodeView(float threshold) {

	threshold_ = threshold;

	//ノードタイトルの設定
	setTitle("ScoreCondition");
	//ノードの色の設定
	setStyle(ImFlow::NodeStyle::green());

	//入力ピンの設定
	//親ノードから実行フロー
	auto inPin = addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());
	AddInputPin(inPin.get());

	//リーフノードなので出力ピンなし

}

//====================================================================
// ノードの内容描画
//====================================================================
void ScoreConditionNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: スコア閾値の表示
	if (logicNode_) {
		ImGui::Text("Threshold: %.2f", threshold_);
	}
	else {
		// 紐づくロジックノードがない場合はエディタ上で編集可能にする
		ImGui::DragFloat("Threshold", &threshold_, 0.1f);
	}
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void ScoreConditionNodeView::SaveParameters(BehaviorNodeData& data) const {
	BehaviorNodeView::SaveParameters(data);
	data.conditionThreshold = threshold_;
}

void ScoreConditionNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	threshold_ = data.conditionThreshold;
}
