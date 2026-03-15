#include "ScoreConditionNodeView.h"
#include "engine/Base/ImGuiManager.h"

//====================================================================
// コンストラクタ
//====================================================================
ScoreConditionNodeView::ScoreConditionNodeView(float threshold) {

	threshold_ = threshold;

	//ノードタイトルの設定
	setTitle("ScoreCondition");

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
	ImGui::Text("Threshold: %.2f", threshold_);
}
