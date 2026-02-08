#pragma once
#include "engine/Input/InputData.h"
#include "engine/Math/Vector2.h"
#include "engine/Base/SpriteManager.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>


// 前方宣言
class SpriteCommon;
class Sprite;

//============================================================================
// ActionButtonICon class
//============================================================================
class ActionButtonICon {
public:
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ActionButtonICon() = default;
	~ActionButtonICon() = default;

	//========================================================================
	// functions
	//========================================================================

	// 初期化
	void Initialize(TakeC::SpriteManager* spriteManager, const Vector2& position,
		GamepadButtonType targetButton);
	// 更新
	void Update();
	// 描画
	void Draw();
	// ImGuiの更新
	void UpdateImGui([[maybe_unused]] const std::string& name);

private:
	TakeC::SpriteManager* spriteManager_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	// アイコン表示用スプライト
	Sprite* sprite_;

	// 監視するボタン
	GamepadButtonType targetButton_;

	// 基準サイズ（押していないときのサイズ）
	Vector2 baseSize_ = { 64.0f, 64.0f };

	// 押し込み時の縮小率
	float pressScale_ = 0.9f;
};