#include "PhaseMessageUI.h"
#include "engine/math/Easing.h"
#include "engine/Animation/SpriteAnimation.h"

//====================================================================
//			初期化
//====================================================================

void PhaseMessageUI::Initialize() {
	// フェーズメッセージスプライトの生成
	phaseMessageText_ = std::make_unique<Sprite>();
	phaseMessageText_->Initialize(SpriteCommon::GetInstance(), "UI/PhaseMessage_ReadyText.png");
	phaseMessageText_->AdjustTextureSize();
	phaseMessageText_->SetPosition({ 640.0f,100.0f });
	phaseMessageText_->SetAnchorPoint({ 0.5f,0.5f });

	// 帯スプライトの生成
	bandSprite_ = std::make_unique<Sprite>();
	bandSprite_->Initialize(SpriteCommon::GetInstance(),"UI/PhaseMessageBand.png");
	bandSprite_->AdjustTextureSize();
	bandSprite_->SetPosition({ 640.0f,100.0f });
	bandSprite_->SetAnchorPoint({ 0.5f,0.5f });

	// タイマー初期化
	displayTimer_.Reset();

}

//====================================================================
//			更新処理
//====================================================================

void PhaseMessageUI::Update() {
	// タイマー更新
	displayTimer_.Update();

	
	// タイマーが終了していなければアルファ値を減少させる
	if (!displayTimer_.IsFinished()) {
		SpriteAnim::UpScale(*bandSprite_, { 1280.0f,0.0f }, { 1280.0f,80.0f }, 1.0f, Easing::EasingType::OUT_BACK, SpriteAnim::PlayMode::ONCE);
		color_.w = 1.0f - displayTimer_.GetProgress();
	} else {
		color_.w = 0.0f;

	}
	// スプライトのアルファ値設定
	phaseMessageText_->SetMaterialColor(color_);

	// スプライトの更新
	phaseMessageText_->Update();
	bandSprite_->Update();
}

//====================================================================
//			ImGuiの更新処理
//====================================================================

void PhaseMessageUI::UpdateImGui() {
	phaseMessageText_->UpdateImGui("PhaseMessageText");
	bandSprite_->UpdateImGui("BandSprite");
}

//====================================================================
//			描画処理
//====================================================================

void PhaseMessageUI::Draw() {
	phaseMessageText_->Draw();
	bandSprite_->Draw();
}
