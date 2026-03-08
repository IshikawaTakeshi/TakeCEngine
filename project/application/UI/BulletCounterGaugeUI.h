#pragma once
#include "engine/2d/BaseUI.h"
#include "engine/Math/Vector2.h"
#include "engine/Math/Vector4.h"
#include <string>

namespace TakeC {
	class SpriteManager;
}

class Sprite;

//============================================================================
// BulletCounterGaugeUI class
//============================================================================
class BulletCounterGaugeUI : public BaseUI {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BulletCounterGaugeUI() = default;
	~BulletCounterGaugeUI() override = default;

	//================================================================
	// functions
	//================================================================

	// 初期化
	void Initialize(TakeC::SpriteManager* spriteManager) override;

	// 更新
	void Update() override;

	// 比率設定
	void SetRatio(float ratio);

	// 色設定
	void SetColor(const Vector4& color);

	// 座標の設定
	void SetPosition(const Vector2& position) override;

private:
	Sprite* frameSprite_ = nullptr; // ゲージの外枠
	Sprite* barSprite_ = nullptr;   // ゲージのバー
	float ratio_ = 1.0f;            // 残弾比率
	float maxWidth_ = 0.0f;         // ゲージの最大幅
};
