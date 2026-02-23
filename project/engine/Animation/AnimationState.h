#pragma once
#include "engine/Animation/NodeAnimation.h"
#include <cmath>

//============================================================================
// AnimationState struct
//============================================================================
struct AnimationState {

	Animation* animation = nullptr; //アニメーションへの非所有ポインタ
	float time = 0.0f;              //現在の再生時間

	/// <summary>
	/// 有効かどうか
	/// </summary>
	bool IsValid() const { return animation != nullptr; }

	/// <summary>
	/// 再生時間をリセット
	/// </summary>
	void Reset() { time = 0.0f; }

	/// <summary>
	/// 再生時間を進める（ループ）
	/// </summary>
	void Advance(float dt) {
		if (animation && animation->duration > 0.0f) {
			time = std::fmod(time + dt, animation->duration);
		}
	}
};
