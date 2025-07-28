#pragma once
#include "engine/2d/Sprite.h"
#include <memory>
#include <string>

/////////////////////
//シーン遷移クラス
/////////////////////
class SceneTransition {
public:

	enum class TransitionState {
		NONE,
		FADE_IN,
		FADE_OUT,
	};

public:

	static SceneTransition* GetInstance();

	//初期化
	void Initialize();
	//終了処理
	void Finalize();
	//更新処理
	void Update();
	//描画処理
	void Draw();
	//シーン遷移開始
	void Start(TransitionState state, float duration);
	//シーン遷移を中止させる処理
	void Stop();

	//シーン遷移が終了しているか
	bool IsFinished();

private:

	static SceneTransition* instance_;

	SceneTransition() = default;
	~SceneTransition() = default;
	SceneTransition(SceneTransition&) = delete;
	SceneTransition& operator=(SceneTransition&) = delete;

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