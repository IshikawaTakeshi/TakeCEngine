#include "WeightSelectorNodeView.h"

//====================================================================
// コンストラクタ
//====================================================================
WeightSelectorNodeView::WeightSelectorNodeView() {

	//ノードタイトルの設定
	setTitle("WeightSelector");

	//入力ピンの設定
	//親ノードから実行フロー
	addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());

	//出力ピンの設定
	//子ノードへの実行フロー
	auto out = addOUT<BehaviorStatus>("Child0");
	out->behaviour([]() {return BehaviorStatus::Invalid; }); //TODO: 子ノードの状態を返すように実装
}

//====================================================================
// ノードの内容描画
//====================================================================
void WeightSelectorNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	//子ピンの追加・削除ボタン
	if (ImGui::Button("Add Child")) {
		std::string name = "Child" + std::to_string(childCount_);
		auto out = addOUT<BehaviorStatus>(name);
		out->behaviour([]() {return BehaviorStatus::Invalid; }); //TODO: 子ノードの状態を返すように実装
		childCount_++;
	}
}
