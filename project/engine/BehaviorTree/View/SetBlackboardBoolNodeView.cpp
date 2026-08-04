#include "SetBlackboardBoolNodeView.h"
#include <imgui.h>

//====================================================================
// コンストラクタ
//====================================================================
SetBlackboardBoolNodeView::SetBlackboardBoolNodeView(const std::string& key, bool value)
	: bbKey_(key), bbValue_(value) {

	//ノードタイトルの設定
	setTitle("SetBlackboardBool");
	//ノードの色の設定(アクションと同じ色)
	setStyle(ImFlow::NodeStyle::blue());

	//入力ピンの設定（親ノードから実行フロー）
	auto inPin = addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());
	AddInputPin(inPin.get());

	//リーフノードなので出力ピンなし
}

//====================================================================
// ノードの内容描画
//====================================================================
void SetBlackboardBoolNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: キーと値の変更UI
	ImGui::PushItemWidth(120.0f);

	char buffer[256];
	strncpy_s(buffer, bbKey_.c_str(), sizeof(buffer));
	if (ImGui::InputText("Key", buffer, sizeof(buffer))) {
		bbKey_ = buffer;
	}

	ImGui::Checkbox("Value", &bbValue_);

	ImGui::PopItemWidth();
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void SetBlackboardBoolNodeView::SaveParameters(BehaviorNodeData& data) const {
	BehaviorNodeView::SaveParameters(data);
	data.bbKey = bbKey_;
	data.bbValue = bbValue_;
}

void SetBlackboardBoolNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	bbKey_ = data.bbKey;
	bbValue_ = data.bbValue;
}
