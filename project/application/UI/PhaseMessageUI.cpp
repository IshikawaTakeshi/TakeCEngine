#include "PhaseMessageUI.h"
#include "engine/Animation/SpriteAnimation.h"
#include "engine/base/TextureManager.h"
#include "engine/base/WinApp.h"
#include "engine/math/Easing.h"

//====================================================================
//			初期化
//====================================================================

void PhaseMessageUI::Initialize() {
	// 画面中央（論理座標）
	screenCenter_ = { TakeC::WinApp::kDebugScreenWidth_ / 2.0f,
					 TakeC::WinApp::kDebugScreenHeight_ / 2.0f };

	TakeC::TextureManager::GetInstance().LoadTexture(
		"UI/PhaseMessage_ReadyText.png", false);
	TakeC::TextureManager::GetInstance().LoadTexture(
		"UI/PhaseMessage_FightText.png", false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/GameClearText.png",
		false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/GameOverText.png",
		false);

	

	// 帯スプライトの生成
	bandSprite_ = std::make_unique<Sprite>();
	bandSprite_->Initialize(&SpriteCommon::GetInstance(),
		"UI/PhaseMessageBand.png");
	bandSprite_->AdjustTextureSize();
	bandSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	bandSprite_->SetTranslate(screenCenter_);

	// 帯のサイズ設定（論理座標1280相当を各解像度へ）
	originalBandSizeMin_ = {
		bandSprite_->GetSize().x * TakeC::WinApp::widthPercent_, 0.0f };
	originalBandSizeMax_ = { 
		1280.0f * TakeC::WinApp::widthPercent_,
		bandSprite_->GetSize().y *TakeC::WinApp::heightPercent_
	};

	// フェーズメッセージスプライトの生成
	phaseMessageText_ = std::make_unique<Sprite>();
	phaseMessageText_->Initialize(&SpriteCommon::GetInstance(),
		"UI/PhaseMessage_ReadyText.png");
	phaseMessageText_->AdjustTextureSize();
	phaseMessageText_->SetAnchorPoint({ 0.5f, 0.5f });
	phaseMessageText_->SetTranslate(screenCenter_);

	// 元のサイズを取得し、描画解像度に合わせてスケーリング
	Vector2 scaledTextSize = {
		phaseMessageText_->GetSize().x * TakeC::WinApp::widthPercent_,
		phaseMessageText_->GetSize().y * TakeC::WinApp::heightPercent_ };
	originalTextSizeMin_ = { scaledTextSize.x, 0.0f };
	originalTextSizeMax_ = scaledTextSize;

	nextMessage_ = PhaseMessage::READY;
}

//====================================================================
//			更新処理
//====================================================================

void PhaseMessageUI::Update() {

	// メッセージ切り替え処理
	if (nextMessage_) {

		currentMessage_ = nextMessage_.value();

		// フェーズメッセージスプライトのテクスチャ切り替え
		switch (currentMessage_) {
		case PhaseMessage::READY:

			// 帯アニメーション再生(拡大)
			bandSprite_->Animation()->PlayUpScale(
				originalBandSizeMin_, originalBandSizeMax_, 0.5f, 1.0f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG_ONCE);
			// テキストスプライト拡大
			phaseMessageText_->Animation()->PlayUpScale(
				originalTextSizeMin_, originalTextSizeMax_, 0.5f, 0.5f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG_ONCE);

			break;
		case PhaseMessage::FIGHT:
		case PhaseMessage::WIN:
		case PhaseMessage::LOSE:

			// 帯アニメーション再生(拡大)
			bandSprite_->Animation()->PlayUpScale(
				originalBandSizeMin_, originalBandSizeMax_, 0.5f, 1.5f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG_ONCE);
			// テキストスプライト拡大
			phaseMessageText_->Animation()->PlayUpScale(
				originalTextSizeMin_, originalTextSizeMax_, 0.5f, 1.5f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG_ONCE);
			break;

		default:
			break;
		}

		nextMessage_ = std::nullopt;
	}

	// メッセージごとの更新処理
	switch (currentMessage_) {
	case PhaseMessage::READY:

		isMessageFinished_ = phaseMessageText_->Animation()->IsFinished();
		if (isMessageFinished_) {
			nextMessage_ = PhaseMessage::FIGHT;
		}

		break;
	case PhaseMessage::FIGHT:

		break;
	}

	// スプライト更新
	bandSprite_->Update();
	phaseMessageText_->Update();
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
	bandSprite_->Draw();
	phaseMessageText_->Draw();
}

//====================================================================
//			次のメッセージ設定
//====================================================================
void PhaseMessageUI::SetNextMessage(PhaseMessage message) {
	// 次のメッセージを設定
	nextMessage_ = message;
	// phaseMessageTextにphaseごとのテクスチャを設定
	switch (message) {
	case PhaseMessage::READY:
		phaseMessageText_->SetFilePath("UI/PhaseMessage_ReadyText.png");
		break;
	case PhaseMessage::FIGHT:
		phaseMessageText_->SetFilePath("UI/PhaseMessage_FightText.png");
		break;
	case PhaseMessage::WIN:
		phaseMessageText_->SetFilePath("UI/GameClearText.png");
		break;
	case PhaseMessage::LOSE:
		phaseMessageText_->SetFilePath("UI/GameOverText.png");
		break;
	}

	// テクスチャサイズを調整し、スケーリングを再適用
	phaseMessageText_->AdjustTextureSize();
	Vector2 scaledTextSize = {
		phaseMessageText_->GetSize().x * TakeC::WinApp::widthPercent_,
		phaseMessageText_->GetSize().y * TakeC::WinApp::heightPercent_ };
	originalTextSizeMin_ = { scaledTextSize.x, 0.0f };
	originalTextSizeMax_ = scaledTextSize;
}