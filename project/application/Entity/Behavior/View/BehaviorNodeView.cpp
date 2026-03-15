#include "BehaviorNodeView.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"

void BehaviorNodeView::draw() {
	ImVec4 color = statusToColor(lastStatus_);
	ImGui::TextColored(color, "Status: %s", StringUtility::EnumToString<BehaviorStatus>(lastStatus_).c_str());
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
