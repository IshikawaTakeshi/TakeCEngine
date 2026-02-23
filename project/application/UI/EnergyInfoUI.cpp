#include "EnergyInfoUI.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　初期化
//===================================================================================
void EnergyInfoUI::Initialize(SpriteCommon* spriteCommon,
	const std::string& backgroundFilePath, const std::string& foregroundFilePath) {

	// 背景スプライトの初期化
	backgroundSprite_ = std::make_unique<Sprite>();
	backgroundSprite_->Initialize(spriteCommon, backgroundFilePath);

	// フォアグラウンドスプライトの初期化
	foregroundSprite_ = std::make_unique<Sprite>();
	foregroundSprite_->Initialize(spriteCommon, foregroundFilePath);

	//点滅用スプライトの初期化
	blinkSprite_ = std::make_unique<Sprite>();
	blinkSprite_->Initialize(spriteCommon, foregroundFilePath);
	blinkSprite_->SetMaterialColor({ 1.0f, 0.0f, 0.0f, 0.0f }); // 赤色に設定

	margin_ = 2.0f; // 枠の太さを設定
	blinkTimer_ = 0.0f; // 点滅タイマー初期化
	blinkSpeed_ = 12.0f; // 点滅スピードを設定
}

//===================================================================================
//　更新
//===================================================================================
void EnergyInfoUI::Update(float currentEnergy, float maxEnergy) {
	currentEnergy = std::max(0.0f, std::min(currentEnergy, maxEnergy));
	float hpRatio = currentEnergy / maxEnergy;

	// 背景サイズ取得
	Vector2 bgSize = backgroundSprite_->GetSize();
	Vector2 bgPos = backgroundSprite_->GetTranslate();

	// 点滅スプライトの位置・サイズを背景スプライトと同じにする
	blinkSprite_->SetTranslate(bgPos);
	blinkSprite_->SetSize(bgSize);

	// フォアグラウンドスプライトの幅・高さを枠の内側に収める
	Vector2 fgSize;
	fgSize.x = (bgSize.x - 2 * margin_) * hpRatio; // 横幅はHP割合
	fgSize.y = bgSize.y - 2 * margin_; // 縦幅は枠内

	foregroundSprite_->SetSize(fgSize);

	// 前景スプライトの左上を枠の内側に配置
	Vector2 fgPos = bgPos + Vector2{ margin_, margin_ };
	foregroundSprite_->SetTranslate(fgPos);

	//オーバーヒート中の場合、点滅スプライトの色を赤色に点滅させる
	if (isOverHeating_) {
		blinkTimer_ += TakeCFrameWork::GetDeltaTime();
		float alpha = Easing::EaseOutSine(blinkTimer_ * blinkSpeed_);
		blinkSprite_->SetMaterialColor({ 1.0f, 0.0f, 0.0f, alpha }); // 赤色に設定

	} else {
		blinkSprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, 0.0f }); // 通常は透明に設定
	}

	// スプライトの更新
	backgroundSprite_->Update();
	blinkSprite_->Update();
	foregroundSprite_->Update();
}

//===================================================================================
//　描画
//===================================================================================
void EnergyInfoUI::Draw() {
	backgroundSprite_->Draw();
	blinkSprite_->Draw();
	foregroundSprite_->Draw();
}

//===================================================================================
//　ImGuiの更新
//===================================================================================
void EnergyInfoUI::UpdateImGui([[maybe_unused]] std::string name) {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::SeparatorText("Energy Info UI Settings");
	backgroundSprite_->UpdateImGui(name + "bg Sprite");
	foregroundSprite_->UpdateImGui(name + "fg Sprite");
	blinkSprite_->UpdateImGui(name + "blink Sprite");
#endif
}

// positionの取得
const Vector2& EnergyInfoUI::GetTranslate() const { return position_; }
// sizeの取得
const Vector2& EnergyInfoUI::GetSize() const { return backgroundSprite_->GetSize(); }
// オーバーヒート状態の取得
bool EnergyInfoUI::GetOverHeatState() const { return isOverHeating_; }

// positionの設定
void EnergyInfoUI::SetPosition(const Vector2& position) {
	position_ = position;
	backgroundSprite_->SetTranslate(position_);
	foregroundSprite_->SetTranslate(position_);
}

// sizeの設定
void EnergyInfoUI::SetSize(const Vector2& size) {

	backgroundSprite_->SetSize(size);
	Vector2 foregroundSize = size;
	// 比率を保持
	foregroundSize.x *= foregroundSprite_->GetSize().x / backgroundSprite_->GetSize().x;
	// 枠の内側に収める
	foregroundSprite_->SetSize(foregroundSize);
}

// オーバーヒート状態の設定
void EnergyInfoUI::SetOverHeatState(bool isOverHeating) { isOverHeating_ = isOverHeating; }
// オーバーヒートタイマーの設定
void EnergyInfoUI::SetOverHeatTimer(float timer) { overHeatTimer_ = timer; }
