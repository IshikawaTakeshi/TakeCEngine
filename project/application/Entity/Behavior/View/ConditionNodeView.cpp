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
	value_ = value;

	//ノードタイトルの設定
	setTitle("Condition");

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

	// ノード固有の描画: 条件パラメータの表示
	ImGui::Text("Field: %s", field_.c_str());
	ImGui::Text("Op: %s", op_.c_str());
	ImGui::Text("Value: %.2f", value_);
}
