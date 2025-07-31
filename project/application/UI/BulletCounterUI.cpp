#include "BulletCounterUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"
#include "application/Weapon/BaseWeapon.h"

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

	digits_.resize(3, 0); // 初期化時に3桁の数字を保持するための配列

	reloadSprite_ = std::make_unique<Sprite>();
	reloadSprite_->Initialize(spriteCommon, "UI/ReloadText.png");
	reloadSprite_->AdjustTextureSize();

	// リロード中のスプライトの更新
	reloadSprite_->SetPosition({ 50.0f, 150.0f });
	reloadSprite_->SetSize({ 200.0f, 50.0f });
}

void BulletCounterUI::Update() {

	digits_[0] = (bulletCount_ / 100) % 10; // 百の位
	digits_[1] = (bulletCount_ / 10) % 10;  // 十の位
	digits_[2] = bulletCount_ % 10;         // 一の位

	for(int i = 0; i < 3; ++i) {
		// スプライトの位置を更新
		bulletCounterSprite_[i]->SetPosition({
			bulletCounterPos_.x + i * (bulletCounterSprite_[i]->GetSize().x - spriteSpace_),
			bulletCounterPos_.y
		});

		SetDigitUV(bulletCounterSprite_[i].get(), digits_[i]);

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

void BulletCounterUI::UpdateImGui([[maybe_unused]] const std::string& name) {

	for(int i = 0; i < 3; ++i) {
		bulletCounterSprite_[i]->UpdateImGui(name + std::to_string(i));
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
	// 現在の弾数を返す
	return bulletCount_;
}

uint32_t BulletCounterUI::GetMaxBulletCount() const {
	// 最大弾数を返す
	return maxBulletCount_;
}

const Vector2& BulletCounterUI::GetBulletCounterPosition() const {
	// 弾数カウンターの位置を返す
	return bulletCounterPos_;
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

void BulletCounterUI::SetBulletCounterPosition(const Vector2& position) {

	bulletCounterPos_ = position;
	for (int i = 0; i < 3; ++i) {
		bulletCounterSprite_[i]->SetPosition({
			bulletCounterPos_.x + i * (bulletCounterSprite_[i]->GetSize().x - spriteSpace_),
			bulletCounterPos_.y
		});
	}
}

void BulletCounterUI::SetDigitUV(Sprite* sprite, int digit) {
	// 0-9の10個の数字が横に並んでいる場合
	float uvWidth =sprite->GetTextureSize().x;
	float uvLeft = digit * uvWidth;
	float uvTop = 0.0f;

	// スプライトにUV座標を設定
	sprite->SetTextureLeftTop({ uvLeft, uvTop });
}