#include "BulletCounterUI.h"
#include "application/Weapon/BaseWeapon.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Base/SpriteManager.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Math/Easing.h"

//===================================================================================
// 　初期化
//===================================================================================
void BulletCounterUI::Initialize(TakeC::SpriteManager *spriteManager,
                                 const Vector2 &position) {
  // BaseUIの初期化
  BaseUI::Initialize(spriteManager);

  bulletCounterPos_ = position;        // スプライトの位置
  counterSpriteSize_ = {15.0f, 16.0f}; // スプライトのサイズ
  reloadSpriteSize_ = {65.0f, 20.0f};  // リロードスプライトのサイズ

  for (int i = 0; i < 3; ++i) {
    // 弾数カウンターのスプライトを3つ作成
    Sprite *sprite =
        CreateAndRegisterSpriteFromJson("BulletCounter_NumText.json");
    sprite->SetTranslate(
        {bulletCounterPos_.x + i * (counterSpriteSize_.x - spriteSpace_),
         bulletCounterPos_.y});
    bulletCounterSprite_.push_back(sprite);

    // 最大弾数カウンターのスプライトを3つ作成
    Sprite *maxSprite =
        CreateAndRegisterSpriteFromJson("BulletCounter_MaxNumText.json");
    maxSprite->SetTranslate(
        {bulletCounterPos_.x + i * (counterSpriteSize_.x - spriteSpace_),
         bulletCounterPos_.y + counterSpriteSize_.y + 5.0f});
    maxBulletCounterSprite_.push_back(maxSprite);
  }

  digits_.resize(3, 0);          // 初期化時に3桁の数字を保持するための配列
  maxBulletDigits_.resize(3, 0); // 最大弾数の桁数を保持するための配列

  // リロード中のスプライトの初期化
  reloadSprite_ = CreateAndRegisterSpriteFromJson("BulletCounter_Reload.json");
  reloadSprite_->SetTranslate(bulletCounterPos_);

  separatorSpriteSize_ = {110.0f, 1.0f}; // セパレータースプライトのサイズ
  // セパレータースプライトの初期化
  separatorSprite_ =
      CreateAndRegisterSpriteFromJson("BulletCounter_Separator.json");
  separatorSprite_->SetTranslate({
      bulletCounterPos_.x + 1.5f * (counterSpriteSize_.x - spriteSpace_),
      bulletCounterPos_.y + counterSpriteSize_.y +
          5.0f // セパレーターの位置を弾数カウンターの中央に設定
  });

  // 武器アイコンのスプライトの初期化
  weaponIconSprite_ =
      CreateAndRegisterSpriteFromJson("BulletCounter_WeaponIcon.json");
  weaponIconSprite_->SetTranslate(
      {bulletCounterPos_.x -
           40.0f, // アイコンの位置を弾数カウンターの左側に設定
       bulletCounterPos_.y});
}

//===================================================================================
// 　更新
//===================================================================================
void BulletCounterUI::Update() {
  BaseUI::Update();
  if (!isActive_)
    return;

  // 現在の弾数と最大弾数のカウンターを更新
  digits_[0] = (bulletCount_ / 100) % 10; // 百の位
  digits_[1] = (bulletCount_ / 10) % 10;  // 十の位
  digits_[2] = bulletCount_ % 10;         // 一の位

  maxBulletDigits_[0] = (remainingBulletCount_ / 100) % 10; // 百の位
  maxBulletDigits_[1] = (remainingBulletCount_ / 10) % 10;  // 十の位
  maxBulletDigits_[2] = remainingBulletCount_ % 10;         // 一の位

  for (int i = 0; i < 3; ++i) {
    // 数字がカウントされてるように見えるよう処理
    SetDigitUV(bulletCounterSprite_[i], digits_[i]);
    // 最大弾数のスプライトも同様に更新
    SetDigitUV(maxBulletCounterSprite_[i], maxBulletDigits_[i]);

    // リロード状態に応じてアルファ値を切り替え
    if (isReloading_) {
      bulletCounterSprite_[i]->SetAlpha(0.0f);
    } else {
      bulletCounterSprite_[i]->SetAlpha(1.0f);
    }
  }

  if (isReloading_) {
    reloadSprite_->SetAlpha(1.0f);
  } else {
    reloadSprite_->SetAlpha(0.0f);
  }
}

//===================================================================================
// 　描画
//===================================================================================
void BulletCounterUI::Draw() {
  BaseUI::Draw();
  // BaseUI由来のスプライト描画は、上位のSpriteManager一括描画で行なうためここでは何もしない
}

//===================================================================================
// 　ImGuiの更新
//===================================================================================
void BulletCounterUI::UpdateImGui([[maybe_unused]] const std::string &name) {
  BaseUI::UpdateImGui(name);
#if defined(_DEBUG) || defined(_DEVELOP)
  for (int i = 0; i < 3; ++i) {
    // bulletCounterSprite_[i]->UpdateImGui(name + std::to_string(i));
  }
  // reloadSprite_->UpdateImGui(name + "_Reload");
  // weaponIconSprite_->UpdateImGui(name + "_WeaponIcon");
  ImGui::Begin("BulletCounterUI");
  ImGui::Text("Bullet Count: %d", bulletCount_);
  ImGui::Checkbox("Is Reloading", &isReloading_);
  ImGui::DragFloat("spriteSpace", &spriteSpace_, 0.1f, 0.0f, 20.0f);
  ImGui::End();
#endif
}

uint32_t BulletCounterUI::GetBulletCount() const {
  // 現在の弾数を返す
  return bulletCount_;
}

uint32_t BulletCounterUI::GetRemainingBulletCount() const {
  // 残弾数の取得
  return remainingBulletCount_;
}

const Vector2 &BulletCounterUI::GetBulletCounterPosition() const {
  // 弾数カウンターの位置を返す
  return bulletCounterPos_;
}

void BulletCounterUI::SetBulletCount(uint32_t count) { bulletCount_ = count; }

void BulletCounterUI::SetRemainingBulletCount(uint32_t count) {
  // 残りの弾数を設定
  remainingBulletCount_ = count;
}

void BulletCounterUI::SetReloadingState(bool isReloading) {
  isReloading_ = isReloading;
}

//===================================================================================
// 　弾数カウンターの位置設定
//===================================================================================
void BulletCounterUI::SetBulletCounterPosition(const Vector2 &position) {

  bulletCounterPos_ = position;
  for (int i = 0; i < 3; ++i) {
    bulletCounterSprite_[i]->SetTranslate(
        {bulletCounterPos_.x +
             i * (bulletCounterSprite_[i]->GetSize().x - spriteSpace_),
         bulletCounterPos_.y});
  }
}

//===================================================================================
// 　武器アイコンのUV設定
//===================================================================================
void BulletCounterUI::SetWeaponIconUV(int weaponIndex) {

  float uvWidth = weaponIconSprite_->GetTextureSize().x;
  float uvLeft = weaponIndex * uvWidth;
  float uvTop = 0.0f;

  // スプライトにUV座標を設定
  weaponIconSprite_->SetTextureLeftTop({uvLeft, uvTop});
}

//===================================================================================
// 　数字のUV設定
//===================================================================================
void BulletCounterUI::SetDigitUV(Sprite *sprite, int digit) {
  // 0-9の10個の数字が横に並んでいる場合
  float uvWidth = sprite->GetTextureSize().x;
  float uvLeft = digit * uvWidth;
  float uvTop = 0.0f;

  // スプライトにUV座標を設定
  sprite->SetTextureLeftTop({uvLeft, uvTop});
}