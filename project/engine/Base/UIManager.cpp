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
	for (auto& ui : uiList_) {
		if (ui->IsActive()) {
			ui->Update();
		}
	}
}

//============================================================================
// 全UIの描画
//============================================================================
void UIManager::Draw() {
	// 登録されている全UIを描画
	// (BaseUI::Draw() は内部でSpriteManagerを呼ばない仕様にした場合、
	//  ここでの呼び出しは実質何もしないか、個別の特殊描画用となる)
	for (auto& ui : uiList_) {
		if (ui->IsActive()) {
			ui->Draw();
		}
	}
}

//============================================================================
// ImGuiの一括更新
//============================================================================
void UIManager::UpdateImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("UI Manager")) {

		int i = 0;
		for (auto& ui : uiList_) {
			// 各UIのImGuiを表示
			// UIクラス側で識別名を付けられるようにしておくと便利
			// ここでは簡易的に連番か、型名がわかればそれを表示したいところ
			std::string name = "UI_" + std::to_string(i++);
			ui->UpdateImGui(name);
		}

		ImGui::TreePop();
	}
#endif
}

//============================================================================
// 既存のUIインスタンスを登録する（所有権移動）
//============================================================================
void UIManager::RegisterUI(std::unique_ptr<BaseUI> ui) {
	if (ui) {
		uiList_.push_back(std::move(ui));
	}
}

//============================================================================
// 全UIのクリア
//============================================================================
void UIManager::Clear() {
	uiList_.clear();
}