#include "BulletCounterUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

void BulletCounterUI::Initialize(SpriteCommon* spriteCommon) {
	bulletCounterSprite_ = std::make_unique<Sprite>();
	bulletCounterSprite_->Initialize(spriteCommon, "UI/BulletCounter.png");
	bulletCounterSprite_->AdjustTextureSize();
	
	WeaponPositionSprite_ = std::make_unique<Sprite>();
	WeaponPositionSprite_->Initialize(spriteCommon, "UI/WeaponPosition.png");
	WeaponPositionSprite_->AdjustTextureSize();
	
	reloadSprite_ = std::make_unique<Sprite>();
	reloadSprite_->Initialize(spriteCommon, "UI/Reload.png");
	reloadSprite_->AdjustTextureSize();

	// 弾数カウンターの更新
	bulletCounterSprite_->SetPosition({ 50.0f, 50.0f });
	bulletCounterSprite_->SetSize({ 200.0f, 50.0f });

	// 武器位置スプライトの更新
	WeaponPositionSprite_->SetPosition({ 50.0f, 100.0f });
	WeaponPositionSprite_->SetSize({ 200.0f, 50.0f });

	// リロード中のスプライトの更新
	reloadSprite_->SetPosition({ 50.0f, 150.0f });
	reloadSprite_->SetSize({ 200.0f, 50.0f });
}

void BulletCounterUI::Update() {
	
}

void BulletCounterUI::Draw() {
	bulletCounterSprite_->Draw();
	WeaponPositionSprite_->Draw();
	reloadSprite_->Draw();
}

void BulletCounterUI::UpdateImGui() {}

uint32_t BulletCounterUI::GetBulletCount() const {
	return bulletCount_;
}

uint32_t BulletCounterUI::GetMaxBulletCount() const {
	return maxBulletCount_;
}

void BulletCounterUI::SetBulletCount(uint32_t count) {
	bulletCount_ = count;
}

void BulletCounterUI::SetMaxBulletCount(uint32_t count) {
	maxBulletCount_ = count;
}

void BulletCounterUI::SetReloadingState(bool isReloading) {
	isReloading_ = isReloading;
}

void BulletCounterUI::SetReloadTimer(float timer) {
	reloadTimer_ = timer;
}
