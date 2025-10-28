#pragma once
#include "TakeCFrameWork.h"
#include "Weapon/Bullet/BulletManager.h"
#include <chrono>

//============================================================================
// MyGame class
//============================================================================
class MyGame : public TakeCFrameWork {
public:

	MyGame() = default;
	~MyGame() = default;

	//========================================================================
	// functions
	//========================================================================

	//初期化
	void Initialize(const std::wstring& titleName) override;
	//終了処理
	void Finalize() override;
	//更新処理
	void Update()override;
	//描画処理
	void Draw()override;

public:

	//時間倍率のリクエスト
	static void RequestTimeScale(float timeScale, float duration,float current);

private:

	//========================================================================
	//リソース読み込み関数群
	//========================================================================

	//モデル読み込み
	void LoadModel();
	//アニメーション読み込み
	void LoadAnimation();
	//テクスチャ読み込み
	void LoadTexture();
	//サウンド読み込み
	void LoadSound();
	//パーティクルプリセット読み込み
	void LoadParticlePreset();

private:

	//現在のシーン
	std::shared_ptr<BaseScene> currentScene_;
	//タイムスケールのリクエスト値
	static float requestedTimeScale_;
	//タイムスケール変更用タイマー
	static Timer timeScaleTimer_;
};