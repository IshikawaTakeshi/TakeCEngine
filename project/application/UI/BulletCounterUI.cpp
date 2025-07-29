#include "BulletCounterUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

void BulletCounterUI::Initialize(SpriteCommon* spriteCommon) {

	bulletCounterPos_ = { 750.0f, 500.0f }; // スプライトの位置を設定
	for (int i = 0; i < 3; ++i) {
		std::unique_ptr<Sprite> sprite;
		sprite = std::make_unique<Sprite>();
		sprite->Initialize(spriteCommon, "UI/numText.png");
		sprite->AdjustTextureSize();
		sprite->SetTextureSize({ 72.0f,72.0f });
		sprite->SetSize({ 32.0f, 32.0f }); // スプライトのサイズを設定

		sprite->SetPosition({
			bulletCounterPos_.x + i * (sprite->GetSize().x - spriteSpace_),
			bulletCounterPos_.y  // 弾数カウンターの位置を設定
			}
		);

		bulletCounterSprite_.push_back(std::move(sprite));
	}


	//WeaponPositionSprite_[0] = std::make_unique<Sprite>();
	//WeaponPositionSprite_[0]->Initialize(spriteCommon, "UI/WeaponPosition.png");
	//WeaponPositionSprite_[0]->AdjustTextureSize();

	reloadSprite_ = std::make_unique<Sprite>();
	reloadSprite_->Initialize(spriteCommon, "UI/ReloadText.png");
	reloadSprite_->AdjustTextureSize();

	// リロード中のスプライトの更新
	reloadSprite_->SetPosition({ 50.0f, 150.0f });
	reloadSprite_->SetSize({ 200.0f, 50.0f });
}

void BulletCounterUI::Update() {

	for(int i = 0; i < 3; ++i) {
		// スプライトの位置を更新
		bulletCounterSprite_[i]->SetPosition({
			bulletCounterPos_.x + i * (bulletCounterSprite_[i]->GetSize().x - spriteSpace_),
			bulletCounterPos_.y
		});

		// スプライトのテクスチャを更新
		bulletCounterSprite_[i]->Update();
	}

}

void BulletCounterUI::Draw() {
	for (int i = 0; i < 3; ++i) {
		bulletCounterSprite_[i]->Draw();
	}
	reloadSprite_->Draw();
}

void BulletCounterUI::UpdateImGui() {

	for(int i = 0; i < 3; ++i) {
		bulletCounterSprite_[i]->UpdateImGui("BulletCounterSprite" + std::to_string(i));
	}
	ImGui::Begin("BulletCounterUI");
	ImGui::Text("Bullet Count: %d", bulletCount_);
	ImGui::Text("Max Bullet Count: %d", maxBulletCount_);
	ImGui::Checkbox("Is Reloading", &isReloading_);
	if (isReloading_) {
		ImGui::Text("Reload Timer: %.2f", reloadTimer_);
	} else {
		ImGui::Text("Not Reloading");
	}
	ImGui::DragFloat("spriteSpace", &spriteSpace_, 0.1f, 0.0f, 20.0f);
	ImGui::End();
}

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
