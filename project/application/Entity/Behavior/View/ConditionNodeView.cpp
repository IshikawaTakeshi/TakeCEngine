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
		// ロジックノードがある場合はそのインスペクタに任せるが、ノード上にも情報を出す
		ImGui::Text("Field: %s", field_.c_str());
		ImGui::Text("Op: %s", op_.c_str());
		ImGui::Text("Threshold: %.2f", threshold);
	}
	else {
		// 紐づくロジックノードがない場合（または初期状態）はエディタ上で直接編集可能にする
		ImGui::PushItemWidth(100.0f);
		
		char fieldBuf[128];
		strncpy_s(fieldBuf, field_.c_str(), sizeof(fieldBuf));
		if (ImGui::InputText("Field", fieldBuf, sizeof(fieldBuf))) {
			field_ = fieldBuf;
		}

		const char* ops[] = { ">=", "<=", ">", "<", "==", "!=" };
		int currentOp = 0;
		for (int i = 0; i < 6; ++i) {
			if (op_ == ops[i]) {
				currentOp = i;
				break;
			}
		}
		if (ImGui::Combo("Op", &currentOp, ops, 6)) {
			op_ = ops[currentOp];
		}

		ImGui::DragFloat("Value", &threshold, 0.1f);

		ImGui::PopItemWidth();
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
