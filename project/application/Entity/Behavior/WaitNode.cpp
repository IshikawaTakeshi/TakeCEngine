#include "WaitNode.h"
#include "engine/Base/TakeCFrameWork.h"
#include <imgui.h>

//==================================================================================
// コンストラクタ
//==================================================================================
WaitNode::WaitNode(float waitTime, const std::string& name)
	: waitTime_(waitTime) {
	name_ = name;
}

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus WaitNode::Execute(Blackboard&) {
	elapsedTime_ += TakeCFrameWork::GetDeltaTime();

	if (elapsedTime_ >= waitTime_) {
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
void WaitNode::Reset() {
	BehaviorNode::Reset();
	elapsedTime_ = 0.0f;
}

//==================================================================================
// ImGuiを用いたデバッグ用情報の描画
//==================================================================================
void WaitNode::DrawInspector() {
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

	// 待機時間
	ImGui::SetNextItemWidth(kWidgetWidth);
	ImGui::DragFloat("Wait Time [s]", &waitTime_, 0.1f, 0.0f, 600.0f, "%.2f");

	// 経過時間（読み取り専用）
	ImGui::Text("Elapsed: %.2f / %.2f", elapsedTime_, waitTime_);

	ImGui::PopID();
#endif
}
