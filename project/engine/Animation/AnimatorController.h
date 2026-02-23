#pragma once
#include "engine/Animation/AnimationState.h"
#include "engine/Animation/Skeleton.h"

//============================================================================
// AnimatorController class
//============================================================================
class AnimatorController {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="skeleton">スケルトンへのポインタ</param>
	void Initialize(Skeleton* skeleton);

	/// <summary>
	/// アニメーション遷移の開始
	/// </summary>
	/// <param name="animation">遷移先アニメーション</param>
	/// <param name="blendDuration">ブレンドにかかる時間（秒）</param>
	void TransitionTo(Animation* animation, float blendDuration);

	/// <summary>
	/// 更新（スケルトンにブレンド済みアニメーションを適用）
	/// </summary>
	/// <param name="dt">デルタタイム</param>
	void Update(float dt);

private:

	Skeleton* skeleton_ = nullptr;     //スケルトンへの非所有ポインタ
	AnimationState currentState_;      //現在のアニメーション状態
	AnimationState nextState_;         //遷移先のアニメーション状態
	float blendTimer_ = 0.0f;         //ブレンド経過時間
	float blendDuration_ = 0.0f;      //ブレンド時間
	bool isBlending_ = false;         //ブレンド中かどうか
};
