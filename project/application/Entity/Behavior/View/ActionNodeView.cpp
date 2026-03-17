#include "ActionNodeView.h"

//====================================================================
// コンストラクタ
//====================================================================
ActionNodeView::ActionNodeView(const std::string& stateName) {

	targetStateName_ = stateName;

	//ノードタイトルの設定
	setTitle("Action:" + targetStateName_);
	//ノードの色の設定
	setStyle(ImFlow::NodeStyle::red());

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
void ActionNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: ターゲットステート名の表示・編集
	ImGui::Text("Target State:", targetStateName_.c_str());
}
