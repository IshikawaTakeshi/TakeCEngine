#pragma once
#include "engine/Animation/AnimationState.h"

class Skeleton;

//============================================================================
// AnimatorController
//============================================================================
class AnimatorController {
public:
	AnimatorController() = default;
	~AnimatorController() = default;

	/// <summary>
	/// 初期化。対象のSkeletonを設定する
	/// </summary>
	void Initialize(Skeleton* skeleton);

	/// <summary>
	/// 毎フレーム呼ぶ。アニメ時間を進め、ブレンドしてSkeletonに適用する
	/// </summary>
	/// <param name="dt">デルタタイム（秒）</param>
	void Update(float dt);

	/// <summary>
	/// アニメーションの遷移をリクエストする
	/// </summary>
	void TransitionTo(Animation* animation, float blendDuration = 0.2f);

	//-------------------------------------------------------------------
	// accessors
	//-------------------------------------------------------------------

	/// <summary>
	/// アニメーターが有効かどうか（animationが1つ以上セットされている）
	/// </summary>
	bool IsValid() const { return currentState_.IsValid(); }

	/// <summary>
	/// 現在の再生時間取得
	/// </summary>
	float GetCurrentTime() const { return currentState_.time; }

private:
	Skeleton* skeleton_ = nullptr;

	AnimationState currentState_; // 現在再生中のアニメ
	AnimationState nextState_;    // ブレンド先のアニメ

	float blendTimer_ = 0.0f;    // ブレンド経過時間
	float blendDuration_ = 0.0f; // ブレンド全体時間
	bool isBlending_ = false;    // ブレンド中フラグ
};
