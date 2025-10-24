#pragma once
#include "engine/math/Vector2.h"
#include <memory>
#include <string>

// 前方宣言
class SpriteCommon;
class Sprite;

//============================================================================
// EnergyInfoUI class
//============================================================================
class EnergyInfoUI {
public:

	EnergyInfoUI() = default;
	~EnergyInfoUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(float currentEnergy, float maxEnergy);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui([[maybe_unused]] std::string name);

	//=============================================================================
	// getter
	//=============================================================================

	/// 位置を取得
	const Vector2& GetTranslate() const;
	/// サイズを取得
	const Vector2& GetSize() const;
	/// オーバーヒート状態を取得
	bool GetOverHeatState() const;

	//==============================================================================
	// setter
	//==============================================================================

	/// 位置を設定
	void SetPosition(const Vector2& position);
	/// サイズを設定
	void SetSize(const Vector2& size);
	/// オーバーヒート状態を設定
	void SetOverHeatState(bool isOverHeating);
	/// オーバーヒートタイマーを設定
	void SetOverHeatTimer(float timer);

private:

	std::unique_ptr<Sprite> backgroundSprite_; // 背景スプライト
	std::unique_ptr<Sprite> blinkSprite_;      // 点滅用スプライト
	std::unique_ptr<Sprite> foregroundSprite_; // 前景スプライト
	Vector2 position_ = { 0.0f, 0.0f };          // HPバーの位置

	// アウトライン（枠）の太さ
	float margin_ = 2.0f;

	float overHeatTimer_ = 0.0f; // オーバーヒートタイマー
	bool isOverHeating_ = false; // オーバーヒート状態フラグ

	//点滅タイマー
	float blinkTimer_ = 0.0f;
	//点滅スピード
	float blinkSpeed_ = 0.0f;
};