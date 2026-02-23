#pragma once
#include "engine/Animation/NodeAnimation.h"
#include <cmath>

//============================================================================
// AnimationState
// アニメーションの再生状態（アニメポインタ + 再生時間）を保持する軽量構造体
//============================================================================
struct AnimationState {

	Animation* animation = nullptr; // 対象アニメーション（非所有）
	float time = 0.0f;              // 現在の再生時間（秒）

	/// <summary>
	/// 再生時間を進める（ループあり）
	/// </summary>
	void Advance(float dt) {
		if (!animation) {
			return;
		}
		time += dt;
		if (animation->duration > 0.0f) {
			time = std::fmod(time, animation->duration);
		}
	}

	/// <summary>
	/// アニメーションが有効かどうか
	/// </summary>
	bool IsValid() const { return animation != nullptr; }

	/// <summary>
	/// リセット
	/// </summary>
	void Reset() { time = 0.0f; }
};
