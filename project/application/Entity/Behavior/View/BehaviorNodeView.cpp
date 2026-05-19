#include "BehaviorNodeView.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"
#include "application/Entity/Behavior/BehaviorNode.h"

//====================================================================
// ノードの内容描画
//====================================================================
void BehaviorNodeView::draw() {
	if (nodeCustomName_ == "UnnamedNode" || nodeCustomName_.empty()) {
		nodeCustomName_ = getName();
	}

	ImVec4 color = statusToColor(currentStatus_);
	ImGui::TextColored(color, "Status: %s", StringUtility::EnumToString<BehaviorStatus>(currentStatus_).c_str());

	// ノード名の編集
	char nameBuf[128];
	strncpy_s(nameBuf, nodeCustomName_.c_str(), sizeof(nameBuf) - 1);
	nameBuf[sizeof(nameBuf) - 1] = '\0';
	ImGui::SetNextItemWidth(120.0f);
	if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
		nodeCustomName_ = nameBuf;
		setTitle(nodeCustomName_);
		if (logicNode_) {
			logicNode_->SetName(nodeCustomName_);
		}
	}

	// ロジックノードのインスペクタを表示
	if (logicNode_) {
		logicNode_->DrawInspector();
	}
}

ImVec4 BehaviorNodeView::statusToColor(BehaviorStatus status) {
	
	switch (status) {
	case BehaviorStatus::Invalid:
		return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // グレー
	case BehaviorStatus::Success:
		return ImVec4(0.1f, 1.0f, 0.1f, 1.0f); // 緑
	case BehaviorStatus::Failure:
		return ImVec4(1.0f, 0.1f, 0.1f, 1.0f); // 赤
	case BehaviorStatus::Running:
		return ImVec4(1.0f, 1.0f, 0.1f, 1.0f); // 黄色
	case BehaviorStatus::Aborted:
		return ImVec4(1.0f, 0.5f, 0.1f, 1.0f); // オレンジ
	default:
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 白（デフォルト）
	}
}

ImFlow::Pin* BehaviorNodeView::GetInputPin(size_t index) const {
	if (index < inPins_.size()) {
		return inPins_[index];
	}
	return nullptr;
}

void BehaviorNodeView::AddInputPin(ImFlow::Pin* pin) {
	if (pin) {
		inPins_.push_back(pin);
	}
}

ImFlow::Pin* BehaviorNodeView::GetOutputPin(size_t index) const {
	if (index < outPins_.size()) {
		return outPins_[index];
	}
	return nullptr;
}

void BehaviorNodeView::AddOutputPin(ImFlow::Pin* pin) {
	if (pin) {
		outPins_.push_back(pin);
	}
}
