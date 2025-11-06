#pragma once
#include <functional>
#include "engine/math/Easing.h"
#include "engine/math/Vector2.h"
#include "engine/Utility/Timer.h"



//前方宣言
class Sprite;

//============================================================================
// SpriteAnimator class
//============================================================================
class SpriteAnimator {
public:

	///状態列挙型
	enum class State {
		None,
		Up,
		Delay,
		Down
	};

	///再生モード列挙型
	enum class PlayMode {
		LOOP,
		ONCE,
		PINGPONG
	};

public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SpriteAnimator() = default;
	~SpriteAnimator() = default;

	/// <summary>
	/// 拡大アニメーションの再生
	/// </summary>
	/// <param name="target"></param>
	/// <param name="startSize"></param>
	/// <param name="endSize"></param>
	/// <param name="duration"></param>
	/// <param name="delay"></param>
	/// <param name="easeType"></param>
	/// <param name="playMode"></param>
	void PlayUpScale(const Vector2& startSize, const Vector2& endSize, float duration, float delay, Easing::EasingType easeType, PlayMode playMode);

	void Initialize(Sprite* target);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime"></param>
	void Update(float deltaTime);

	bool IsPlaying() const { return state_ != State::None; }

	bool IsFinished() const { return state_ == State::None; }

private:
	Sprite* target_ = nullptr;
	State state_ = State::None;
	PlayMode playMode_ = PlayMode::ONCE;

	Vector2 startSize_{};
	Vector2 endSize_{};

	float duration_ = 1.0f;
	float delay_ = 0.0f;
	float timer_ = 0.0f;

	Easing::EasingType easeType_ = Easing::EasingType::LINEAR;

	// 現在実行するアニメーション関数（ラムダで格納）
	std::function<void(float)> currentAnimFunc_;
};