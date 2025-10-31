#pragma once
#include <memory>
#include "engine/2d/Sprite.h"
#include "engine/Utility/Timer.h"

//============================================================================
// PushStartUI class
//============================================================================
class PushStartUI {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	PushStartUI() = default;
	~PushStartUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:

	//========================================================================
	// variables
	//========================================================================
	
	// 「PRESS START」テキストスプライト
	std::unique_ptr<Sprite> pressStartText_ = nullptr;
	// 点滅スプライト
	std::unique_ptr<Sprite> blinkSprite_ = nullptr;

	// 点滅タイマー
	Timer blinkTimer_;
	// アルファ値
	float alpha_ = 1.0f;
};