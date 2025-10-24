#pragma once
#include "engine/math/Vector2.h"
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
	void Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath);

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
	Vector2 position_ = { 0.0f, 0.0f };          // HPバーの位置

	// アウトライン（枠）の太さ
	float margin_ = 2.0f;
	
	bool isActive_ = true; // アクティブ状態
	float alpha_ = 1.0f; // アルファ値（透明度）
	float fadeSpeed_ = 1.0f; // フェード速度
	float timer_ = 0.0f; // タイマー

};