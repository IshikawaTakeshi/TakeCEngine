#include "ActionButtonICon.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/Input/Input.h"

using namespace TakeC;

void ActionButtonICon::Initialize(TakeC::SpriteManager* spriteManager,
	const Vector2& position,
	GamepadButtonType targetButton) {
	spriteManager_ = spriteManager;
	targetButton_ = targetButton;

	sprite_ = spriteManager_->Create("GamePad_KeyIcon.png");
	// 座標と初期サイズの設定
	sprite_->SetTranslate(position);
	sprite_->SetSize(baseSize_);

	// アンカーポイントを中心にしておくと、縮小時に中心に向かって縮むので自然に見えます
	sprite_->SetAnchorPoint({ 0.5f, 0.5f });

	targetButton_ = targetButton;
}

void ActionButtonICon::Update() {
	// シングルトンからInputインスタンスを取得
	Input& input = Input::GetInstance();

	int stickNo = 0;
	if (input.PushButton(stickNo, targetButton_)) {
		// --- ボタンが押されているとき ---

		// 1. サイズを少し小さくする
		sprite_->SetSize({ baseSize_.x * pressScale_, baseSize_.y * pressScale_ });

		// 2. 色を少し暗くする (RGBA: 0.7, 0.7, 0.7, 1.0)
		sprite_->SetMaterialColor({ 0.7f, 0.7f, 0.7f, 1.0f });
	} else {
		// --- ボタンが押されていないとき ---

		// サイズを元に戻す
		sprite_->SetSize(baseSize_);

		// 色を白（通常）に戻す
		sprite_->SetMaterialColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	// スプライト自体の更新はManagerが行うため不要
}

void ActionButtonICon::Draw() {
	// 描画
	// SpriteManagerが一括描画・破棄管理するため、ここでのDrawは不要
}

void ActionButtonICon::UpdateImGui(const std::string& name) {
	sprite_->UpdateImGui(name);
}
