#include "BulletCounterUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"
#include "application/Weapon/BaseWeapon.h"

void BulletCounterUI::Initialize(SpriteCommon* spriteCommon,const Vector2& position) {

	bulletCounterPos_ = position; // スプライトの位置
	counterSpriteSize_ = { 24.0f, 24.0f }; // スプライトのサイズ
	reloadSpriteSize_ = { 80.0f, 24.0f }; // リロードスプライトのサイズ

	for (int i = 0; i < 3; ++i) {
		std::unique_ptr<Sprite> sprite;
		sprite = std::make_unique<Sprite>();
		sprite->Initialize(spriteCommon, "UI/numText.png");
		sprite->AdjustTextureSize();
		sprite->SetTextureSize({ 72.0f,72.0f });
		sprite->SetSize(counterSpriteSize_); // スプライトのサイズを設定
		sprite->SetPosition({
			bulletCounterPos_.x + i * (sprite->GetSize().x - spriteSpace_),
			bulletCounterPos_.y  // 弾数カウンターの位置を設定
			}
		);

		bulletCounterSprite_.push_back(std::move(sprite));

		std::unique_ptr<Sprite> maxSprite;
		maxSprite = std::make_unique<Sprite>();
		maxSprite->Initialize(spriteCommon, "UI/numText.png");
		maxSprite->AdjustTextureSize();
		maxSprite->SetTextureSize({ 72.0f,72.0f });
		maxSprite->SetSize(counterSpriteSize_); // スプライトのサイズを設定
		maxSprite->SetPosition({
			bulletCounterPos_.x + i * (maxSprite->GetSize().x - spriteSpace_),
			bulletCounterPos_.y + counterSpriteSize_.y + 5.0f // 弾数カウンターの下に配置
			}
		);
		maxBulletCounterSprite_.push_back(std::move(maxSprite));
	}

	digits_.resize(3, 0); // 初期化時に3桁の数字を保持するための配列
	maxBulletDigits_.resize(3, 0); // 最大弾数の桁数を保持するための配列

	reloadSprite_ = std::make_unique<Sprite>();
	reloadSprite_->Initialize(spriteCommon, "UI/ReloadText.png");
	reloadSprite_->AdjustTextureSize();

	// リロード中のスプライトの更新
	reloadSprite_->SetPosition(bulletCounterPos_);
	reloadSprite_->SetSize(reloadSpriteSize_);
	reloadSprite_->GetMesh()->GetMaterial()->SetMaterialColor({ 1.0f, 0.2f, 0.2f, 1.0f }); // 赤色に設定
}

void BulletCounterUI::Update() {

	// 現在の弾数と最大弾数のカウンターを更新
	digits_[0] = (bulletCount_ / 100) % 10; // 百の位
	digits_[1] = (bulletCount_ / 10) % 10;  // 十の位
	digits_[2] = bulletCount_ % 10;         // 一の位

	maxBulletDigits_[0] = (remainingBulletCount_ / 100) % 10; // 百の位
	maxBulletDigits_[1] = (remainingBulletCount_ / 10) % 10;  // 十の位
	maxBulletDigits_[2] = remainingBulletCount_ % 10;         // 一の位

	for(int i = 0; i < 3; ++i) {
		//数字がカウントされてるように見えるよう処理
		SetDigitUV(bulletCounterSprite_[i].get(), digits_[i]);
		// 最大弾数のスプライトも同様に更新
		SetDigitUV(maxBulletCounterSprite_[i].get(), maxBulletDigits_[i]);

		// スプライトのテクスチャを更新
		bulletCounterSprite_[i]->Update();
		// 最大弾数のスプライトも更新
		maxBulletCounterSprite_[i]->Update();
	}

	reloadSprite_->Update();

}

void BulletCounterUI::Draw() {
	if (isReloading_ == false) {
		for (int i = 0; i < 3; ++i) {
			bulletCounterSprite_[i]->Draw();

		}
	} else {

		reloadSprite_->Draw();
	}
	for(int i = 0; i < 3; ++i) {
		maxBulletCounterSprite_[i]->Draw();
	}
}

void BulletCounterUI::UpdateImGui([[maybe_unused]] const std::string& name) {

	for(int i = 0; i < 3; ++i) {
		bulletCounterSprite_[i]->UpdateImGui(name + std::to_string(i));
	}
	reloadSprite_->UpdateImGui(name + "_Reload");
	ImGui::Begin("BulletCounterUI");
	ImGui::Text("Bullet Count: %d", bulletCount_);
	ImGui::Checkbox("Is Reloading", &isReloading_);
	ImGui::DragFloat("spriteSpace", &spriteSpace_, 0.1f, 0.0f, 20.0f);
	ImGui::End();
}

uint32_t BulletCounterUI::GetBulletCount() const {
	// 現在の弾数を返す
	return bulletCount_;
}

uint32_t BulletCounterUI::GetRemainingBulletCount() const {
	// 残弾数の取得
	return remainingBulletCount_;
}

const Vector2& BulletCounterUI::GetBulletCounterPosition() const {
	// 弾数カウンターの位置を返す
	return bulletCounterPos_;
}

void BulletCounterUI::SetBulletCount(uint32_t count) {
	bulletCount_ = count;
}

void BulletCounterUI::SetRemainingBulletCount(uint32_t count) {
	// 残りの弾数を設定
	remainingBulletCount_ = count;
}

void BulletCounterUI::SetReloadingState(bool isReloading) {
	isReloading_ = isReloading;
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