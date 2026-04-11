#include "ConditionNodeView.h"

//====================================================================
// コンストラクタ
//====================================================================
ConditionNodeView::ConditionNodeView(
	const std::string& field,
	const std::string& op,
	float value) {

	field_ = field;
	op_ = op;
	threshold = value;

	//ノードタイトルの設定
	setTitle("Condition");
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
void ConditionNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: 条件の表示・編集
	if (logicNode_) {
		ImGui::Text("Field: %s", field_.c_str());
		ImGui::Text("Op: %s", op_.c_str());
		ImGui::Text("Threshold: %.2f", threshold);
	}
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void ConditionNodeView::SaveParameters(BehaviorNodeData& data) const {
	BehaviorNodeView::SaveParameters(data);
	data.field = field_;
	data.op = op_;
	data.conditionThreshold = threshold;
}

void ConditionNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	field_ = data.field;
	op_ = data.op;
	threshold = data.conditionThreshold;
}
