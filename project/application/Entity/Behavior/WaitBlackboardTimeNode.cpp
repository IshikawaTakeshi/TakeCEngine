#include "WaitBlackboardTimeNode.h"
#include "application/Entity/Behavior/Blackboard.h"
#include "engine/Base/TakeCFrameWork.h"
#include <imgui.h>

//==================================================================================
// コンストラクタ
//==================================================================================
WaitBlackboardTimeNode::WaitBlackboardTimeNode(const std::string& bbKey, const std::string& name)
	: bbKey_(bbKey) {
	name_ = name;
}

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus WaitBlackboardTimeNode::Execute(Blackboard& blackboard) {
	// 初回 Execute 時に待機時間をキャッシュ
	if (!isCached_) {
		cachedWaitTime_ = blackboard.HasKey(bbKey_)
			? blackboard.GetValue<float>(bbKey_)
			: 0.0f;
		isCached_ = true;
	}

	elapsedTime_ += TakeCFrameWork::GetDeltaTime();

	if (elapsedTime_ >= cachedWaitTime_) {
		Reset();
		currentStatus_ = BehaviorStatus::Success;
		return BehaviorStatus::Success;
	}

	currentStatus_ = BehaviorStatus::Running;
	return BehaviorStatus::Running;
}

//==================================================================================
// ノードのリセット（再実行時に状態をクリア）
//==================================================================================
void WaitBlackboardTimeNode::Reset() {
	BehaviorNode::Reset();
	elapsedTime_    = 0.0f;
	cachedWaitTime_ = 0.0f;
	isCached_       = false;
}

//==================================================================================
// ImGuiを用いたデバッグ用情報の描画
//==================================================================================
void WaitBlackboardTimeNode::DrawInspector() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::PushID(this);

	static constexpr float kWidgetWidth = 120.0f;

	// 名前
	char nameBuf[128];
	strncpy_s(nameBuf, name_.c_str(), sizeof(nameBuf) - 1);
	ImGui::SetNextItemWidth(kWidgetWidth);
	if (ImGui::InputText("Node Name", nameBuf, sizeof(nameBuf))) {
		name_ = nameBuf;
	}

	// キー名（読み取り専用表示）
	ImGui::Text("BB Key: %s", bbKey_.c_str());

	// 実行時デバッグ情報
	if (isCached_) {
		ImGui::Text("Wait Time: %.2f", cachedWaitTime_);
		ImGui::Text("Elapsed:   %.2f", elapsedTime_);
	} else {
		ImGui::TextDisabled("(not started)");
	}

	ImGui::PopID();
#endif
}
