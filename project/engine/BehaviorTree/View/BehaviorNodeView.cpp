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

	// コンテンツ領域の開始Y座標を記録（縦サイズ計算に使用）
	contentStartY_ = ImGui::GetCursorPosY();

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

	//=========================================================
	// [EXT] ユーザー指定サイズの適用（Dummyで領域を確保）
	//=========================================================
	if (userSize_.x > 0.f || userSize_.y > 0.f) {
		// 横: userSize_.x を絶対値でそのまま Dummy に渡す
		//     (差分方式だとフィードバックループで振動するため)
		float dummyW = (userSize_.x > 0.f) ? userSize_.x : 0.f;

		// 縦: コンテンツが消費した高さを実測し、残りを Dummy で確保する
		//     GetCursorPosY() の差分を使うことで getSize() 依存のループを回避
		float contentUsedH = ImGui::GetCursorPosY() - contentStartY_;
		float dummyH = (userSize_.y > 0.f) ? std::max(0.f, userSize_.y - contentUsedH) : 0.f;

		ImGui::Dummy(ImVec2(dummyW, dummyH));
	}

	//=========================================================
	// [EXT] リサイズハンドル（ノード右下の三角グリップ）
	//=========================================================
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 paddingBR = { getStyle()->padding.z, getStyle()->padding.w };
		ImVec2 offset = getHandler()->grid2screen({ 0.f, 0.f });
		ImVec2 br = offset + getPos() + getSize() + paddingBR;
		float handleSz = 10.0f;
		ImVec2 handleMin = ImVec2(br.x - handleSz, br.y - handleSz);

		// グリップ三角形を描画
		drawList->AddTriangleFilled(
			br,
			ImVec2(handleMin.x, br.y),
			ImVec2(br.x, handleMin.y),
			IM_COL32(180, 180, 180, 120));

		// ホバー時・リサイズ中にカーソル変更
		bool onHandle = ImGui::IsMouseHoveringRect(handleMin, br);
		if (onHandle || resizing_) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
		}

		// ドラッグ開始（右下ホバー時はリサイズを優先）
		if (onHandle && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			resizing_ = true;
			// userSize が未設定なら現在のサイズで初期化
			if (userSize_.x <= 0.f) userSize_.x = getSize().x;
			if (userSize_.y <= 0.f) userSize_.y = getSize().y;
		}

		// ドラッグ中のリサイズ
		if (resizing_) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
			ImVec2 delta = ImGui::GetIO().MouseDelta;
			userSize_.x = std::max(50.0f, userSize_.x + delta.x);
			userSize_.y = std::max(30.0f, userSize_.y + delta.y);
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				resizing_ = false;
			}
		}
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
