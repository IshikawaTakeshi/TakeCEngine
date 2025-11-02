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

	nextMessage_ = PhaseMessage::READY;
}

//====================================================================
//			更新処理
//====================================================================

void PhaseMessageUI::Update() {
	

	if (nextMessage_) {

		currentMessage_ = nextMessage_.value();

		switch (currentMessage_) {
		case PhaseMessage::READY:
			
			// 帯アニメーション再生(拡大)
			bandSprite_->Animation()->PlayUpScale(
				{ bandSprite_->GetSize().x,0.0f },
				{ bandSprite_->GetSize().x, bandSprite_->GetSize().y },
				0.5f,
				1.0f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG
			);	

			break;
		case PhaseMessage::GO:
			
			bandSprite_->Animation()->PlayUpScale(
				{ bandSprite_->GetSize().x,0.0f },
				{ bandSprite_->GetSize().x, bandSprite_->GetSize().y },
				0.3f,
				0.5f,
				Easing::EasingType::OUT_QUAD,
				SpriteAnimator::PlayMode::PINGPONG
			);
			break;
		
		default:
			break;
		}

		nextMessage_ = std::nullopt;
	}

	switch (currentMessage_) {
	case PhaseMessage::READY:

		bool isBandAnimFinished = !bandSprite_->Animation()->IsFinished();
		if(!isBandAnimFinished) {
			nextMessage_ = PhaseMessage::GO;
		}

		break;

	}

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
