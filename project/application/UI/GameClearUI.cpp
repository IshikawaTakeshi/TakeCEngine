#include "GameClearUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

void GameClearUI::Initialize(SpriteCommon* spriteCommon) {

	spriteCommon_ = spriteCommon;

	textSprite_ = std::make_unique<Sprite>();
	textSprite_->Initialize(spriteCommon, "game_clear_text.png");
	isActive_ = false;
}

void GameClearUI::Update() {
	if (!isActive_) return;
	// タイマー更新
	timer_ += TakeCFrameWork::GetDeltaTime();
	// アルファ値の更新
	if (timer_ < displayTime_) {
		alpha_ += fadeSpeed_ * TakeCFrameWork::GetDeltaTime();
	} else {
		alpha_ -= fadeSpeed_ * TakeCFrameWork::GetDeltaTime();
	}
	// アルファ値の制限
	alpha_ = std::clamp(alpha_, 0.0f, 1.0f);
	textSprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, alpha_ });
	textSprite_->Update();
}

void GameClearUI::Draw() {
	if (!isActive_) return;
	textSprite_->Draw();
}

void GameClearUI::SetIsActive(bool isActive) {
	isActive_ = isActive;
	if (isActive_) {
		timer_ = 0.0f; // タイマーをリセット
		alpha_ = 0.0f; // アルファ値を初期化
		textSprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, alpha_ }); // 初期状態で透明に設定
	}
}