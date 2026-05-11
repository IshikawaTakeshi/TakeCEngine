#pragma once
#include "engine/2d/BaseUI.h"

//============================================================================
// EnergyInfoUI class
//============================================================================
class EnergyInfoUI : public BaseUI {
public:

	EnergyInfoUI() = default;
	~EnergyInfoUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::SpriteManager* spriteManager);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(float currentEnergy, float maxEnergy);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui(const std::string& name) override;

	//=============================================================================
	// getter
	//=============================================================================

	/// サイズを取得
	const Vector2& GetSize() const;
	/// オーバーヒート状態を取得
	bool GetOverHeatState() const;

	//==============================================================================
	// setter
	//==============================================================================

	/// 位置を設定
	void SetPosition(const Vector2& position) override;
	/// サイズを設定
	void SetSize(const Vector2& size);
	/// オーバーヒート状態を設定
	void SetOverHeatState(bool isOverHeating);
	/// オーバーヒートタイマーを設定
	void SetOverHeatTimer(float timer);

private:

	// アウトライン（枠）の太さ
	float margin_ = 2.5f;

	float overHeatTimer_ = 0.0f; // オーバーヒートタイマー
	bool isOverHeating_ = false; // オーバーヒート状態フラグ

	//点滅タイマー
	float blinkTimer_ = 0.0f;
	//点滅スピード
	float blinkSpeed_ = 12.0f;
};