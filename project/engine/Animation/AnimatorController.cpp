#include "AnimatorController.h"
#include <algorithm>

//====================================================================
// 初期化
//====================================================================
void AnimatorController::Initialize(Skeleton* skeleton) {
	skeleton_ = skeleton;
	currentState_ = {};
	nextState_ = {};
	blendTimer_ = 0.0f;
	blendDuration_ = 0.0f;
	isBlending_ = false;
}

//====================================================================
// アニメーション遷移の開始
//====================================================================
void AnimatorController::TransitionTo(Animation* animation, float blendDuration) {
	if (!animation) { return; }

	if (!currentState_.IsValid()) {
		// 現在アニメーションがない場合は即座に切り替え
		currentState_.animation = animation;
		currentState_.Reset();
		isBlending_ = false;
	} else {
		// ブレンド遷移を開始
		nextState_.animation = animation;
		nextState_.Reset();
		blendDuration_ = std::max(blendDuration, 0.0001f);
		blendTimer_ = 0.0f;
		isBlending_ = true;
	}
}

//====================================================================
// 更新
//====================================================================
void AnimatorController::Update(float dt) {
	if (!skeleton_) { return; }
	if (!currentState_.IsValid()) { return; }

	currentState_.Advance(dt);

	if (isBlending_) {
		nextState_.Advance(dt);
		blendTimer_ += dt;
		float blend = std::clamp(blendTimer_ / blendDuration_, 0.0f, 1.0f);

		skeleton_->ApplyBlendedAnimation(
			currentState_.animation, currentState_.time,
			nextState_.animation, nextState_.time,
			blend);

		if (blend >= 1.0f) {
			// ブレンド完了：次の状態を現在の状態に昇格
			currentState_ = nextState_;
			nextState_ = {};
			isBlending_ = false;
		}
	} else {
		// ブレンドなし：現在のアニメーションをそのまま適用
		skeleton_->ApplyBlendedAnimation(
			currentState_.animation, currentState_.time,
			nullptr, 0.0f,
			0.0f);
	}

	skeleton_->Update();
}
