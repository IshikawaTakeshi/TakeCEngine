#include "UIManager.h"

#include "engine/Base/ImGuiManager.h"
#include "engine/Base/SpriteManager.h"

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================
void UIManager::Initialize(SpriteManager* spriteManager) {
	spriteManager_ = spriteManager;
	Clear();
}

//============================================================================
// 全UIの更新
//============================================================================
void UIManager::Update() {
	// 登録されている全UIを更新
	for (auto& pair : uiMap_) {
		if (pair.second->IsActive()) {
			pair.second->Update();
		}
	}
}

//============================================================================
// 全UIの描画
//============================================================================
void UIManager::Draw() {
	// 登録されている全UIを描画
	for (auto& pair : uiMap_) {
		if (pair.second->IsActive()) {
			pair.second->Draw();
		}
	}
}

//============================================================================
// ImGuiの一括更新
//============================================================================
void UIManager::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::Begin("UI Manager");

	for (auto& pair : uiMap_) {
		// 各UIのImGuiを表示 (登録名を識別用に使用)
		pair.second->UpdateImGui(pair.first);
	}

	ImGui::End();
#endif
}

//============================================================================
// 既存のUIインスタンスを登録する（所有権移動）
//============================================================================
void UIManager::RegisterUI(const std::string& name, std::unique_ptr<BaseUI> ui) {
	if (ui) {
		uiMap_[name] = std::move(ui);
	}
}

//============================================================================
// 全UIのクリア
//============================================================================
void UIManager::Clear() {
	uiMap_.clear();
}

//============================================================================
// 特定のUIのアクティブ状態を設定する
//============================================================================
void TakeC::UIManager::SetUIActive(const std::string& name, bool isActive) {
	auto it = uiMap_.find(name);
	if (it != uiMap_.end()) {
		it->second->SetActive(isActive);
	}
}
