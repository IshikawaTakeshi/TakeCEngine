#include "SetBlackboardStringNodeView.h"
#include <imgui.h>

//====================================================================
// コンストラクタ
//====================================================================
SetBlackboardStringNodeView::SetBlackboardStringNodeView(const std::string& key, const std::string& value)
	: bbKey_(key), bbStringValue_(value) {

	// ノードタイトルの設定
	setTitle("SetBlackboardString");
	// アクションノードと同じ色
	setStyle(ImFlow::NodeStyle::blue());

	// 入力ピンの設定（親ノードから実行フロー）
	auto inPin = addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());
	AddInputPin(inPin.get());

	// リーフノードなので出力ピンなし
}

//====================================================================
// ノードの内容描画
//====================================================================
void SetBlackboardStringNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: キーと値の変更UI
	ImGui::PushItemWidth(160.0f);

	char keyBuf[256];
	strncpy_s(keyBuf, bbKey_.c_str(), sizeof(keyBuf));
	if (ImGui::InputText("Key", keyBuf, sizeof(keyBuf))) {
		bbKey_ = keyBuf;
	}

	char valueBuf[256];
	strncpy_s(valueBuf, bbStringValue_.c_str(), sizeof(valueBuf));
	if (ImGui::InputText("Value", valueBuf, sizeof(valueBuf))) {
		bbStringValue_ = valueBuf;
	}

	ImGui::PopItemWidth();
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void SetBlackboardStringNodeView::SaveParameters(BehaviorNodeData& data) const {
	BehaviorNodeView::SaveParameters(data);
	data.bbKey = bbKey_;
	data.bbStringValue = bbStringValue_;
}

void SetBlackboardStringNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	bbKey_ = data.bbKey;
	bbStringValue_ = data.bbStringValue;
}
