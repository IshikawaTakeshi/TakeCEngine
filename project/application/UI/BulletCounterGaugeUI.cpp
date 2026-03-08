#include "BulletCounterGaugeUI.h"
#include "engine/2d/Sprite.h"
#include "engine/Base/SpriteManager.h"
#include <algorithm>

//===================================================================================
// 初期化
//===================================================================================
void BulletCounterGaugeUI::Initialize(TakeC::SpriteManager* spriteManager) {
	// BaseUIの初期化
	BaseUI::Initialize(spriteManager);

	// スプライトの生成
	frameSprite_ =
		CreateAndRegisterSpriteFromJson("BulletCounterGauge_Frame.json");
	barSprite_ = CreateAndRegisterSpriteFromJson("BulletCounterGauge_Bar.json");

	// 最大幅の保存
	if (barSprite_) {
		maxWidth_ = barSprite_->GetSize().x;
	}
}

//===================================================================================
// 更新
//===================================================================================
void BulletCounterGaugeUI::Update() {
	BaseUI::Update();
	if (!isActive_)
		return;

	// ゲージの長さを比率に合わせて更新
	if (barSprite_) {
		Vector2 size = barSprite_->GetSize();
		size.x = maxWidth_ * ratio_;
		barSprite_->SetSize(size);
	}
}

//===================================================================================
// 比率設定
//===================================================================================
void BulletCounterGaugeUI::SetRatio(float ratio) {
	ratio_ = std::clamp(ratio, 0.0f, 1.0f);
}

//===================================================================================
// 色設定
//===================================================================================
void BulletCounterGaugeUI::SetColor(const Vector4& color) {
	if (barSprite_) {
		barSprite_->SetMaterialColor(color);
	}
}

//===================================================================================
// 座標の設定
//===================================================================================
void BulletCounterGaugeUI::SetPosition(const Vector2& position) {
	BaseUI::SetPosition(position);

	if (frameSprite_) {
		frameSprite_->SetTranslate(position);
	}
	if (barSprite_) {
		// BarはFrameに対してオフセットがある可能性があるため、
		// JSONでの初期位置をオフセットとして扱う
		Vector2 offset = { -maxWidth_ / 2.0f, 0.0f }; // 左端合わせの想定
		barSprite_->SetTranslate({ position.x + offset.x, position.y + offset.y });
	}
}
