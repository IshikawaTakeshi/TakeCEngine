#include "PushStartUI.h"

//====================================================================
//			初期化
//====================================================================
void PushStartUI::Initialize() {
	// 「PRESS START」テキストスプライトの生成
	pressStartText_ = std::make_unique<Sprite>();
	pressStartText_->Initialize(SpriteCommon::GetInstance(), "UI/PressStart.png");
	pressStartText_->AdjustTextureSize();
	pressStartText_->SetTranslate({ 512.0f, 500.0f });
	pressStartText_->SetAnchorPoint({ 0.5f,0.5f });
	pressStartText_->SetSize({ 400.0f,60.0f });
	// 点滅スプライトの生成
	blinkSprite_ = std::make_unique<Sprite>();
	blinkSprite_->Initialize(SpriteCommon::GetInstance(), "UI/TitleSelect.png");
	blinkSprite_->SetTranslate({ 512.0f, 500.0f });
	blinkSprite_->SetAnchorPoint({ 0.5f,0.5f });
	blinkSprite_->SetSize({ 400.0f,60.0f });
	// 点滅タイマーの初期化
	blinkTimer_.Initialize(1.0f, 0.0f);
}

//====================================================================
//			更新処理
//====================================================================
void PushStartUI::Update() {
	// 点滅タイマーの更新
	blinkTimer_.Update();

	// アルファ値の計算（0.0f〜1.0fの範囲で変化）
	float progress = blinkTimer_.GetProgress();

	if (progress <= 0.5f) {
		// 0.0fから0.5fの間はアルファ値を増加
		alpha_ = progress * 2.0f; // 0.0fから1.0fへ
	}
	else {
		// 0.5fから1.0fの間はアルファ値を減少
		alpha_ = (1.0f - progress) * 2.0f; // 1.0fから0.0fへ
	}

	// 点滅スプライトのアルファ値を設定
	blinkSprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, alpha_ });
	// タイマーが終了したらリセットしてループさせる
	if (blinkTimer_.IsFinished()) {
		blinkTimer_.Reset();
	}

	// スプライトの更新
	pressStartText_->Update();
	blinkSprite_->Update();
}

//====================================================================
//			ImGuiの更新処理
//====================================================================
void PushStartUI::UpdateImGui() {
	pressStartText_->UpdateImGui("PressStartText");
	blinkSprite_->UpdateImGui("BlinkSprite");
}

//====================================================================
//			描画処理
//====================================================================
void PushStartUI::Draw() {
	// 「PRESS START」テキストスプライトの描画
	pressStartText_->Draw();
	// 点滅スプライトの描画
	blinkSprite_->Draw();
}