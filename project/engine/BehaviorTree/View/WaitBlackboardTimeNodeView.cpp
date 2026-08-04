#include "WaitBlackboardTimeNodeView.h"
#include "application/Entity/Behavior/WaitBlackboardTimeNode.h"
#include <imgui.h>

//====================================================================
// コンストラクタ
//====================================================================
WaitBlackboardTimeNodeView::WaitBlackboardTimeNodeView(const std::string& bbKey)
	: bbKey_(bbKey) {

	// ノードタイトルの設定
	setTitle("WaitBlackboardTime");
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
void WaitBlackboardTimeNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	ImGui::PushItemWidth(120.0f);

	if (logicNode_) {
		// ロジックノードがある場合: キーを同期表示
		auto* waitBB = static_cast<WaitBlackboardTimeNode*>(logicNode_);
		bbKey_ = waitBB->GetBBKey();
		ImGui::Text("BB Key: %s", bbKey_.c_str());
	} else {
		// エディタ上でキーを選択
		if (blackboardKeys_.empty()) {
			// Blackboard キーが未登録の場合は直接入力にフォールバック
			char buf[256];
			strncpy_s(buf, bbKey_.c_str(), sizeof(buf));
			if (ImGui::InputText("BB Key", buf, sizeof(buf))) {
				bbKey_ = buf;
			}
		} else {
			// Blackboard キーリストからコンボ選択
			int currentIdx = 0;
			for (int i = 0; i < (int)blackboardKeys_.size(); ++i) {
				if (bbKey_ == blackboardKeys_[i]) {
					currentIdx = i;
					break;
				}
			}

			std::vector<const char*> keyPtrs;
			keyPtrs.reserve(blackboardKeys_.size());
			for (const auto& k : blackboardKeys_) {
				keyPtrs.push_back(k.c_str());
			}

			if (ImGui::Combo("BB Key", &currentIdx, keyPtrs.data(), (int)keyPtrs.size())) {
				bbKey_ = blackboardKeys_[currentIdx];
			}
		}
	}

	ImGui::PopItemWidth();
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void WaitBlackboardTimeNodeView::SaveParameters(BehaviorNodeData& data) const {
	// ロジックノードがある場合は最新値を同期
	if (logicNode_) {
		auto* waitBB = static_cast<WaitBlackboardTimeNode*>(logicNode_);
		const_cast<WaitBlackboardTimeNodeView*>(this)->bbKey_ = waitBB->GetBBKey();
	}
	BehaviorNodeView::SaveParameters(data);
	data.bbKey = bbKey_;
}

void WaitBlackboardTimeNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	bbKey_ = data.bbKey;
}
