#define NOMINMAX
#include "HPBar.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"

HPBar::~HPBar() {
	// unique_ptr により自動解放されるため明示的に削除は不要
}

void HPBar::Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath) {
	// 背景スプライトの初期化
	backgroundSprite_ = std::make_unique<Sprite>();
	backgroundSprite_->Initialize(spriteCommon, backgroundFilePath);

	// フォアグラウンドスプライトの初期化
	foregroundSprite_ = std::make_unique<Sprite>();
	foregroundSprite_->Initialize(spriteCommon, foregroundFilePath);

	margin_ = 2.0f; // 枠の太さを設定
}

void HPBar::Update(float currentHP, float maxHP) {
	// HPの割合を計算

	currentHP = std::max(0.0f, std::min(currentHP, maxHP));
	float hpRatio = currentHP / maxHP;

	// 背景サイズ取得
	Vector2 bgSize = backgroundSprite_->GetSize();
	Vector2 bgPos = backgroundSprite_->GetTranslate();

	// フォアグラウンドスプライトの幅・高さを枠の内側に収める
	Vector2 fgSize;
	fgSize.x = (bgSize.x - 2 * margin_) * hpRatio; // 横幅はHP割合
	fgSize.y = bgSize.y - 2 * margin_; // 縦幅は枠内

	foregroundSprite_->SetSize(fgSize);

	// 前景スプライトの左上を枠の内側に配置
	Vector2 fgPos = bgPos + Vector2{ margin_, margin_ };
	foregroundSprite_->SetPosition(fgPos);

	backgroundSprite_->Update();
	foregroundSprite_->Update();
}

void HPBar::Draw() {
	// 背景スプライトを描画
	backgroundSprite_->Draw();

	// フォアグラウンドスプライトを描画
	foregroundSprite_->Draw();
}

void HPBar::UpdateImGui([[maybe_unused]]std::string name) {

#ifdef _DEBUG
	backgroundSprite_->UpdateImGui(name + "_Background");
	foregroundSprite_->UpdateImGui(name + "_Foreground");
#endif // DEBUG
}



void HPBar::SetPosition(const Vector2& position) {
	position_ = position;

	// 背景スプライトの位置を設定
	backgroundSprite_->SetPosition(position);

	// フォアグラウンドスプライトの位置も背景に揃える
	foregroundSprite_->SetPosition(position);
}

void HPBar::SetSize(const Vector2& size) {
	if (backgroundSprite_) {
		backgroundSprite_->SetSize(size);
	}
	if (foregroundSprite_) {
		Vector2 foregroundSize = size;
		foregroundSize.x *= foregroundSprite_->GetSize().x / backgroundSprite_->GetSize().x; // 比率を保持
		foregroundSprite_->SetSize(foregroundSize);
	}
}

Vector2 HPBar::GetTranslate() const { return position_; }
