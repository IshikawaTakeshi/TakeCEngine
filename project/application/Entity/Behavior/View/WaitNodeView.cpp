#include "WaitNodeView.h"
#include "application/Entity/Behavior/WaitNode.h"
#include <imgui.h>

//====================================================================
// コンストラクタ
//====================================================================
WaitNodeView::WaitNodeView(float waitTime)
	: waitTime_(waitTime) {

	// ノードタイトルの設定
	setTitle("Wait");
	// ノードの色（アクション系と統一）
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
void WaitNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	ImGui::PushItemWidth(100.0f);

	if (logicNode_) {
		// ロジックノードがある場合: 値を同期して表示
		auto* wait = static_cast<WaitNode*>(logicNode_);
		waitTime_ = wait->GetWaitTime();
		ImGui::Text("Wait: %.2f s", waitTime_);
	} else {
		// エディタ上で直接編集（ロジックノード未生成時）
		if (ImGui::DragFloat("Wait [s]", &waitTime_, 0.1f, 0.0f, 600.0f, "%.2f")) {
			// waitTime_ はそのまま保持。SaveParameters で保存される。
		}
	}

	ImGui::PopItemWidth();
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void WaitNodeView::SaveParameters(BehaviorNodeData& data) const {
	// ロジックノードがある場合は最新値を同期
	if (logicNode_) {
		auto* wait = static_cast<WaitNode*>(logicNode_);
		const_cast<WaitNodeView*>(this)->waitTime_ = wait->GetWaitTime();
	}
	BehaviorNodeView::SaveParameters(data);
	data.waitTime = waitTime_;
}

void WaitNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	waitTime_ = data.waitTime;
}
