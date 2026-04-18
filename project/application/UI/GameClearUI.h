#pragma once
#include <memory>
#include <string>

// 前方宣言
class SpriteCommon;
class Sprite;

//============================================================================
// GameClearUI class
//============================================================================
class GameClearUI {
public:
	GameClearUI() = default;
	~GameClearUI() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon);
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	//----- getter ---------------------------

	//----- setter ---------------------------

	//UIの表示状態設定
	void SetIsActive(bool isActive);

private:

	SpriteCommon* spriteCommon_ = nullptr; // スプライト共通クラス

	std::unique_ptr<Sprite> textSprite_; // テキストスプライト

	bool isActive_ = false; // UIの表示状態
	float timer_ = 0.0f; // タイマー
	float displayTime_ = 3.0f; // UIの表示時間
	float fadeSpeed_ = 1.0f; // フェード速度
	float alpha_ = 0.0f; // アルファ値（透明度）
};