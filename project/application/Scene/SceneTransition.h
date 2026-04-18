#pragma once
#include "engine/2d/Sprite.h"
#include <memory>
#include <string>

//============================================================
//	SceneTransition class
//============================================================
class SceneTransition {
private:

	//シングルトンインスタンス
	static SceneTransition* instance_;

	//コンストラクタ・デストラクタ・コピー禁止
	SceneTransition() = default;
	~SceneTransition() = default;
	SceneTransition(SceneTransition&) = delete;
	SceneTransition& operator=(SceneTransition&) = delete;

public:

	//シーン遷移状態列挙型
	enum class TransitionState {
		NONE,
		FADE_IN,
		FADE_OUT,
	};

public:

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static SceneTransition* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	//シーン遷移開始
	void Start(TransitionState state, float duration);
	//シーン遷移を中止させる処理
	void Stop();

	//シーン遷移が終了しているか
	bool IsFinished();

private:

	//シーン遷移状態
	TransitionState state_ = TransitionState::NONE;
	//フェード処理用スプライト
	std::unique_ptr<Sprite> fadeSprite_ = nullptr;

	//遷移時間
	float duration_;
	//経過時間
	float transitionTime_;
	//遷移速度
	float transitionSpeed_;
	//フェードアルファ値
	float alpha_;
};