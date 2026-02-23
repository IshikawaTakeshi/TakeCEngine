#include "ActionButtonICon.h"
#include "engine/Input/Input.h"
#include "engine/base/ImGuiManager.h"

using namespace TakeC;

//======================================================================
//	初期化
//======================================================================
void ActionButtonICon::Initialize(SpriteManager* spriteManager, const std::string& configName,
	PlayerInputProvider* inputProvider, 
	CharacterActionInput targetAction) {

	// 親クラスの初期化
	BaseUI::Initialize(spriteManager);

	inputProvider_ = inputProvider;
	targetAction_ = targetAction;

	if (spriteManager_) {
		sprites_.resize(1);
		sprites_[0] = spriteManager_->CreateFromJson(configName);
		sprites_[0]->SetSizeRelative();
		baseSize_ = sprites_[0]->GetSize();
	}
}

//======================================================================
//	更新処理
//======================================================================
void ActionButtonICon::Update() {

	// 指定されたボタンの入力をチェック
	bool isPressed = false;
	isPressed = inputProvider_->IsActionPressed(targetAction_);

	// 特殊なアクション（ジャンプなど）の場合、RequestJumpInput() を呼ぶ必要があるなら分岐する
	if (targetAction_ == CharacterActionInput::JUMP) {
		isPressed = inputProvider_->RequestJumpInput();
	}

	if (isPressed) {
		sprites_[0]->SetSize({ baseSize_.x * pressScale_, baseSize_.y * pressScale_ });
		sprites_[0]->SetMaterialColor({ 0.7f, 0.7f, 0.7f, 1.0f });
	} else {
		sprites_[0]->SetSize(baseSize_);
		sprites_[0]->SetMaterialColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

//======================================================================
//	ImGuiの更新
//======================================================================
void ActionButtonICon::UpdateImGui(const std::string& name) {
	// 親クラスのImGui（IsActiveやPositionの調整など）
	BaseUI::UpdateImGui(name);

#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui::TreeNode((name + "_Details").c_str())) {
		ImGui::DragFloat2("Base Size", &baseSize_.x);
		ImGui::DragFloat("Press Scale", &pressScale_, 0.01f, 0.1f, 1.0f);
		// ボタンタイプの表示などは必要に応じて追加
		ImGui::TreePop();
	}
#endif
}