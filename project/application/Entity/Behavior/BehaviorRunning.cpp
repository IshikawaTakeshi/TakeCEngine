#include "BehaviorRunning.h"
#include "application/Provider/IMoveDirectionProvider.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Quaternion.h"
#include "engine/math/Easing.h"

BehaviorRunning::BehaviorRunning(IMoveDirectionProvider* provider) {
	moveDirectionProvider_ = provider;
}

void BehaviorRunning::Initialize() {
}

void BehaviorRunning::Update() {

	//移動方向の正規化
	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		moveDirection_ = Vector3Math::Normalize(moveDirection_);
		//移動時の加速度の計算
		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;

		float targetAngle = atan2(moveDirection_.x, moveDirection_.z);
		Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);

		transform_.rotate = Easing::Slerp(transform_.rotate, targetRotate, 0.1f);
		transform_.rotate = QuaternionMath::Normalize(transform_.rotate);
		//パーティクルエミッターの更新
		//backEmitter_->Emit();

	} else {
		//速度の減速処理
		velocity_.x /= deceleration_;
		velocity_.z /= deceleration_;
	}

	//最大移動速度の制限
	float speed = sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed > kMaxMoveSpeed_) {
		float scale = kMaxMoveSpeed_ / speed;
		velocity_.x *= scale;
		velocity_.z *= scale;
	}

	//移動処理
	// 位置の更新（deltaTimeをここで適用）
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;

}
