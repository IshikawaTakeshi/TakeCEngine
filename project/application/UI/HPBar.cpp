#include "HPBar.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　初期化
//===================================================================================
void HPBar::Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath) {
	// 背景スプライトの初期化
	backgroundSprite_ = std::make_unique<Sprite>();
	backgroundSprite_->Initialize(spriteCommon, backgroundFilePath);

	// フォアグラウンドスプライトの初期化
	foregroundSprite_ = std::make_unique<Sprite>();
	foregroundSprite_->Initialize(spriteCommon, foregroundFilePath);

	// ダメージバーの初期化
	damageBarSprite_ = std::make_unique<Sprite>();
	damageBarSprite_->Initialize(spriteCommon, foregroundFilePath);
	damageBarSprite_->SetMaterialColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // ダメージバーは赤色に設定

	margin_ = 2.0f; // 枠の太さを設定

	isActive_ = true; // 初期状態はアクティブ
	alpha_ = 1.0f; // 初期のアルファ値（透明度）
	fadeSpeed_ = 1.0f; // フェード速度を設定

	damageDelayTimer_.Initialize(3.0f, 0.0f); // ダメージバーの遅延タイマーを初期化
}

//===================================================================================
//　更新
//===================================================================================
void HPBar::Update(float currentHP, float maxHP) {
	// HPの割合を計算
	currentHP = std::max(0.0f, std::min(currentHP, maxHP));
	hpRatio_= currentHP / maxHP;

	//
	if (currentHP < previousHP_) {
		// ダメージを受けた場合、遅延タイマーをリセットして開始
		damageDelayTimer_.Initialize(1.5f, 0.0f); // 遅延時間を0.5秒に設定

	}

	// ダメージバーの遅延タイマーを更新
	damageDelayTimer_.Update();

	// ダメージバーの補間処理
	if (damageDelayTimer_.IsFinished()) {
		// 遅延時間が経過したらダメージバーを補間
		damageBarRatio_ = Easing::Lerp(damageBarRatio_, hpRatio_, damageLerpSpeed_ * TakeCFrameWork::GetDeltaTime());
	}

	//旧hpを更新
	previousHP_ = currentHP;

	// 背景サイズ取得
	Vector2 bgSize = backgroundSprite_->GetSize();
	Vector2 bgPos = backgroundSprite_->GetTranslate();

	// フォアグラウンドスプライトの幅・高さを枠の内側に収める
	Vector2 fgSize;
	fgSize.x = (bgSize.x - 2 * margin_) * hpRatio_; // 横幅はHP割合
	fgSize.y = bgSize.y - 2 * margin_; // 縦幅は枠内

	Vector2 damageBarSize;
	damageBarSize.x = (bgSize.x - 2 * margin_) * damageBarRatio_; // 横幅はダメージバー割合
	damageBarSize.y = bgSize.y - 2 * margin_; // 縦幅は枠内

	foregroundSprite_->SetSize(fgSize);
	damageBarSprite_->SetSize(damageBarSize);

	// 前景スプライト・ダメージバーの左上を枠の内側に配置
	Vector2 fgPos = bgPos + Vector2{ margin_, margin_ };
	foregroundSprite_->SetTranslate(fgPos);

	Vector2 damageBarPos = bgPos + Vector2{ margin_, margin_ };
	damageBarSprite_->SetTranslate(damageBarPos);

	//isActiveがfalseになった時
	if( !isActive_) {
		// アルファ値を徐々に減少させる
		alpha_ -= fadeSpeed_ * TakeCFrameWork::GetDeltaTime();
		if (alpha_ <= 0.0f) {
			alpha_ = 0.0f; // 最小値を0に制限
			return; // アクティブでない場合は更新しない
		}
	} else {
		alpha_ = 1.0f; // アクティブな場合はアルファ値を1に設定
	}

	//スプライトの更新
	backgroundSprite_->Update();
	foregroundSprite_->Update();
	damageBarSprite_->Update();
}

//===================================================================================
//　描画
//===================================================================================
void HPBar::Draw() {
	if (!isActive_) return; // アクティブでない場合は描画しない

	// 背景スプライトを描画
	backgroundSprite_->Draw();

	// ダメージバーを描画
	damageBarSprite_->Draw();

	// フォアグラウンドスプライトを描画
	foregroundSprite_->Draw();
}

//===================================================================================
//　ImGuiの更新
//===================================================================================
void HPBar::UpdateImGui([[maybe_unused]]std::string name) {

#ifdef _DEBUG
	backgroundSprite_->UpdateImGui(name + "_Background");
	damageBarSprite_->UpdateImGui(name + "_DamageBar");
	foregroundSprite_->UpdateImGui(name + "_Foreground");
#endif // DEBUG
}

//===================================================================================
//　位置を設定
//===================================================================================
void HPBar::SetPosition(const Vector2& position) {
	position_ = position;

	// 背景スプライトの位置を設定
	backgroundSprite_->SetTranslate(position);

	// ダメージバーの位置も背景に揃える
	damageBarSprite_->SetTranslate(position);

	// フォアグラウンドスプライトの位置も背景に揃える
	foregroundSprite_->SetTranslate(position);
}

//===================================================================================
//　サイズを設定
//===================================================================================
void HPBar::SetSize(const Vector2& size) {
	if (backgroundSprite_) {
		backgroundSprite_->SetSize(size);
	}
	if (foregroundSprite_) {
		Vector2 foregroundSize = size;
		foregroundSize.x *= foregroundSprite_->GetSize().x / backgroundSprite_->GetSize().x; // 比率を保持
		foregroundSprite_->SetSize(foregroundSize);
	}
	if (damageBarSprite_) {
		Vector2 damageBarSize = size;
		damageBarSize.x *= damageBarSprite_->GetSize().x / backgroundSprite_->GetSize().x; // 比率を保持
		damageBarSprite_->SetSize(damageBarSize);
	}
}

//positionの取得
Vector2 HPBar::GetTranslate() const { return position_; }
