#include "AnimatorController.h"
#include "engine/Animation/Skeleton.h"

void AnimatorController::Initialize(Skeleton* skeleton) {

	skeleton_ = skeleton;
	currentState_ = AnimationState(); // 初期状態は無効
	nextState_ = AnimationState();
	blendTimer_ = 0.0f;
	blendDuration_ = 0.0f;
	isBlending_ = false;
}

void AnimatorController::Update(float dt) {


}
