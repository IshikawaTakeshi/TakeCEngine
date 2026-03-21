#include "BulletCounterGaugeUI.h"
#include "engine/2d/Sprite.h"
#include "engine/Base/SpriteManager.h"
#include "engine/Utility/StringUtility.h"
#include <algorithm>

//===================================================================================
// 初期化
//===================================================================================
void BulletCounterGaugeUI::Initialize(TakeC::SpriteManager *spriteManager,
                                      BaseWeapon *currentWeapon) {
  // BaseUIの初期化
  BaseUI::Initialize(spriteManager);
  currentWeapon_ = currentWeapon;
  WeaponUnit weaponUnit = WeaponUnit(currentWeapon_->GetUnitPosition());

  std::string fullPath_Frame = "BulletCounterGauge_Frame_" +
                               StringUtility::EnumToString(weaponUnit) +
                               ".json";
  std::string fullPath_Bar = "BulletCounterGauge_Bar_" +
                             StringUtility::EnumToString(weaponUnit) + ".json";

  // スプライトの生成
  frameSprite_ = CreateAndRegisterSpriteFromJson(fullPath_Frame);
  barSprite_ = CreateAndRegisterSpriteFromJson(fullPath_Bar);

  // 最大幅の保存（解像度に合わせてスケーリング）
  if (barSprite_) {
    maxWidth_ = barSprite_->GetSize().x * TakeC::WinApp::widthPercent_;
  }
}

//===================================================================================
// 更新
//===================================================================================
void BulletCounterGaugeUI::Update() {
  BaseUI::Update();

  // 現在の武器（一応0番目）の状態を反映
  if (currentWeapon_->GetIsReloading()) {
    // リロード中：赤色で進捗を表示
    SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // 赤
    float maxReload = currentWeapon_->GetMaxReloadTime();
    float currentReload = currentWeapon_->GetCurrentReloadTime();
    float ratio = (maxReload > 0.0f) ? (currentReload / maxReload) : 1.0f;
    SetRatio(ratio);
  } else {
    // 通常時：白色で残弾比率を表示
    SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 白
    float ratio = 0.0f;
    if (currentWeapon_->GetMagazineCount() > 0) {
      ratio = static_cast<float>(currentWeapon_->GetBulletCount()) /
              currentWeapon_->GetMagazineCount();
    }
    SetRatio(ratio);
  }

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
void BulletCounterGaugeUI::SetColor(const Vector4 &color) {
  if (barSprite_) {
    barSprite_->SetMaterialColor(color);
  }
}

//===================================================================================
// 座標の設定
//===================================================================================
void BulletCounterGaugeUI::SetPosition(const Vector2 &position) {
  BaseUI::SetPosition(position);

  if (frameSprite_) {
    frameSprite_->SetTranslate(position);
  }
  if (barSprite_) {
    // BarはFrameに対してオフセットがある可能性があるため、
    // JSONでの初期位置をオフセットとして扱う。解像度に合わせてスケーリング
    Vector2 offset = {(-maxWidth_ / 2.0f), 0.0f}; // 左端合わせの想定
    barSprite_->SetTranslate({position.x + offset.x, position.y + offset.y});
  }
}
