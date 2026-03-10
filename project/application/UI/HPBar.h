#pragma once
#include "engine/2d/BaseUI.h"
#include "engine/Math/Vector2.h"
#include "engine/Utility/Timer.h"
#include <memory>
#include <string>

//============================================================================
// HPBar class
//============================================================================
class HPBar : public BaseUI {
public:
	HPBar() = default;
	~HPBar() override = default;

	enum HPBarComponent {
		BACKGROUND, // 背景
		DAMAGE,     // ダメージ（黄色または赤色で遅れて動く分）
		FOREGROUND, // 前景（現在のHP）
		OWNER,      // 名前などの付加情報
		kTotalComponents
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::SpriteManager* spriteManager,
		const std::string& configName);

	/// <summary>
	/// HP情報の更新
	/// </summary>
	void SetHP(float currentHP, float maxHP);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// ImGui
	/// </summary>
	void UpdateImGui(const std::string& name) override;

	/// <summary>
	/// 位置を設定
	/// </summary>
	void SetPosition(const Vector2& position) override;

private:
	// スプライトメンバ
	// (BaseUIのsprites_で管理されるため、ここではポインタのみ保持)
	Sprite* backgroundSprite_ = nullptr;
	Sprite* foregroundSprite_ = nullptr;
	Sprite* damageBarSprite_ = nullptr;
	Sprite* ownerNameSprite_ = nullptr;

	float margin_ =
		2.0f; // 枠の太さ（JSONから取るように拡張も可能だが、現状維持）

	float currentHP_ = 0.0f;  // 現在のHP
	float previousHP_ = 0.0f; // 前回のHP
	float maxHP_ = 0.0f;      // 最大HP
	float hpRatio_ = 1.0f;    // HPの割合

	Timer damageDelayTimer_;       // ダメージバーの遅延タイマー
	float damageBarRatio_ = 1.0f;  // ダメージバーの割合
	float damageLerpSpeed_ = 2.0f; // ダメージバーの補間速度

	float backgroundMaxWidth_ = 0.0f;
	float foregroundMaxWidth_ = 0.0f;
	float damageBarMaxWidth_ = 0.0f;
};