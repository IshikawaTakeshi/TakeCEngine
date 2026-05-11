#include "EnergyInfoUI.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"


//===================================================================================
// 　初期化
//===================================================================================
void EnergyInfoUI::Initialize(TakeC::SpriteManager* spriteManager) {
	// 親の初期化
	BaseUI::Initialize(spriteManager);

	// UIConfigを読み込む
	LoadUIConfig("EnergyInfo");

	margin_ = 2.5f; // 枠の太さ
	blinkTimer_ = 0.0f;          // 点滅タイマー初期化
	blinkSpeed_ = 12.0f;         // 点滅スピードを設定
}

//===================================================================================
// 　更新
//===================================================================================
void EnergyInfoUI::Update(float currentEnergy, float maxEnergy) {
	currentEnergy = std::max(0.0f, std::min(currentEnergy, maxEnergy));
	float hpRatio = currentEnergy / maxEnergy;

	Sprite* backgroundSprite = spriteMap_["Background"];
	Sprite* foregroundSprite = spriteMap_["Foreground"];
	Sprite* blinkSprite = spriteMap_["Blink"];

	if (!backgroundSprite || !foregroundSprite || !blinkSprite) return;

	// 背景サイズ取得
	Vector2 bgSize = backgroundSprite->GetSize();
	Vector2 bgPos = backgroundSprite->GetTranslate();

	// 点滅スプライトの位置・サイズを背景スプライトと同じにする
	blinkSprite->SetTranslate(bgPos);
	blinkSprite->SetSize(bgSize);

	// フォアグラウンドスプライトの幅・高さを枠の内側に収める
	Vector2 fgSize;
	fgSize.x = (bgSize.x - 2 * margin_) * hpRatio; // 横幅はHP割合
	fgSize.y = bgSize.y - 2 * margin_;             // 縦幅は枠内

	foregroundSprite->SetSize(fgSize);

	// 前景スプライトの左上を枠の内側に配置
	Vector2 fgPos = bgPos + Vector2{ margin_, margin_ };
	foregroundSprite->SetTranslate(fgPos);

	// オーバーヒート中の場合、点滅スプライトの色を赤色に点滅させる
	if (isOverHeating_) {
		blinkTimer_ += TakeCFrameWork::GetDeltaTime();
		float alpha = Easing::EaseOutSine(blinkTimer_ * blinkSpeed_);
		blinkSprite->SetMaterialColor({ 1.0f, 0.0f, 0.0f, alpha }); // 赤色に設定

	}
	else {
		blinkSprite->SetMaterialColor(
			{ 1.0f, 1.0f, 1.0f, 0.0f }); // 通常は透明に設定
	}
}

//===================================================================================
// 　描画
//===================================================================================
void EnergyInfoUI::Draw() {
	if (!isActive_) return;

	// BaseUIで管理されているスプライトを順番に描画
	for (auto* sprite : sprites_) {
		sprite->Draw();
	}
}

//===================================================================================
// 　ImGuiの更新
//===================================================================================
void EnergyInfoUI::UpdateImGui(const std::string& name) {
#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui::TreeNode(name.c_str())) {
		BaseUI::UpdateImGui(name);
		ImGui::SeparatorText("Energy Info UI Settings");
		for (auto& pair : spriteMap_) {
			pair.second->UpdateImGui(pair.first);
		}
		ImGui::TreePop();
	}
#endif
}

// sizeの取得
const Vector2& EnergyInfoUI::GetSize() const {
	auto it = spriteMap_.find("Background");
	if (it != spriteMap_.end()) {
		return it->second->GetSize();
	}
	static Vector2 zero = { 0.0f, 0.0f };
	return zero;
}

// オーバーヒート状態の取得
bool EnergyInfoUI::GetOverHeatState() const { return isOverHeating_; }

// positionの設定
void EnergyInfoUI::SetPosition(const Vector2& position) {
	position_ = position;
	// 子要素の位置をオフセットベースで更新するか、Backgroundを基準にするか。
	// ここではBackgroundを直接動かす。
	Sprite* bg = spriteMap_["Background"];
	if (bg) {
		bg->SetTranslate(position_);
	}
}

// sizeの設定
void EnergyInfoUI::SetSize(const Vector2& size) {
	Sprite* bg = spriteMap_["Background"];
	if (bg) {
		bg->SetSize(size);
	}
}

// オーバーヒート状態の設定
void EnergyInfoUI::SetOverHeatState(bool isOverHeating) {
	isOverHeating_ = isOverHeating;
}

// オーバーヒートタイマーの設定
void EnergyInfoUI::SetOverHeatTimer(float timer) { overHeatTimer_ = timer; }
