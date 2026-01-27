#pragma once
#include "engine/math/Vector2.h"
#include "engine/Utility/Timer.h"
#include <memory>
#include <string>

// 前方宣言
class SpriteCommon;
class Sprite;

//============================================================================
// HPBar class
//============================================================================
class HPBar {
public:
	HPBar() = default;
	~HPBar() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon,const std::string& ownerName, const std::string& backgroundFilePath, const std::string& foregroundFilePath);

	/// <summary>
	/// HPの更新
	/// </summary>
	void Update(float currentHP, float maxHP);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui
	/// </summary>
	void UpdateImGui([[maybe_unused]]std::string name);

	/// <summary>
	/// 位置を設定
	/// </summary>
	void SetPosition(const Vector2& position);

	/// <summary>
	/// サイズを設定
	/// </summary>
	void SetSize(const Vector2& size);

	/// <summary>
	/// 位置を取得
	/// </summary>
	Vector2 GetTranslate() const;

private:

	std::unique_ptr<Sprite> backgroundSprite_; // 背景スプライト
	std::unique_ptr<Sprite> foregroundSprite_; // 前景スプライト	
	std::unique_ptr<Sprite> damageBarSprite_; //hpの消費量を表すバー
	std::unique_ptr<Sprite> ownerNameSprite_; // 所有者名スプライト
	std::string ownerNameJsonFile_; // 所有者名
	Vector2 position_ = { 0.0f, 0.0f };          // HPバーの位置

	// アウトライン（枠）の太さ
	float margin_ = 2.0f;
	
	bool isActive_ = true; // アクティブ状態
	float alpha_ = 1.0f; // アルファ値（透明度）
	float fadeSpeed_ = 1.0f; // フェード速度

	float currentHP_ = 0.0f; // 現在のHP
	float previousHP_ = 0.0f; // 前回のHP
	float maxHP_ = 0.0f;     // 最大HP
	float hpRatio_ = 1.0f;    // HPの割合

	Timer damageDelayTimer_; // ダメージバーの遅延タイマー
	float damageBarRatio_ = 1.0f; // ダメージバーの割合
	float damageLerpSpeed_ = 2.0f; // ダメージバーの補間速度

};