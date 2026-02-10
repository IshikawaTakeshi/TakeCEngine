#include "ActionButtonICon.h"
#include "engine/2d/Sprite.h"
#include "engine/Base/SpriteManager.h"
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

		sprite_ = spriteManager_->CreateFromJson(configName);
		baseSize_ = sprite_->GetSize();
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
		sprite_->SetSize({ baseSize_.x * pressScale_, baseSize_.y * pressScale_ });
		sprite_->SetMaterialColor({ 0.7f, 0.7f, 0.7f, 1.0f });
	} else {
		sprite_->SetSize(baseSize_);
		sprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

//======================================================================
//	ImGuiの更新
//======================================================================
void ActionButtonICon::UpdateImGui(const std::string& name) {
	// 親クラスのImGui（IsActiveやPositionの調整など）
	BaseUI::UpdateImGui(name);

#ifdef _DEBUG
	if (ImGui::TreeNode((name + "_Details").c_str())) {
		ImGui::DragFloat2("Base Size", &baseSize_.x);
		ImGui::DragFloat("Press Scale", &pressScale_, 0.01f, 0.1f, 1.0f);
		// ボタンタイプの表示などは必要に応じて追加
		ImGui::TreePop();
	}
#endif
}