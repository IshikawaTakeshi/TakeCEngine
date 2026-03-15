#include "SelectorNodeView.h"

//====================================================================
// コンストラクタ
//====================================================================
SelectorNodeView::SelectorNodeView() {

	//ノードタイトルの設定
	setTitle("Selector");

	//入力ピンの設定
	//親ノードから実行フロー
	auto inPin = addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());
	AddInputPin(inPin.get());

	//出力ピンの設定
	//子ノードへの実行フロー
	auto out = addOUT<BehaviorStatus>("Child0");
	out->behaviour([]() {return BehaviorStatus::Invalid; }); //TODO: 子ノードの状態を返すように実装
	AddOutputPin(out.get());
}

//====================================================================
// ノードの内容描画
//====================================================================
void SelectorNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	//子ピンの追加・削除ボタン
	if (ImGui::Button("Add Child")) {
		std::string name = "Child" + std::to_string(childCount_);
		auto out = addOUT<BehaviorStatus>(name);
		out->behaviour([]() {return BehaviorStatus::Invalid; }); //TODO: 子ノードの状態を返すように実装
		AddOutputPin(out.get());
		childCount_++;
	}
}
