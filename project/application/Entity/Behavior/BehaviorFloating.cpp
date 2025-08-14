#include "BehaviorFloating.h"
#include "engine/io/Input.h"
#include "engine/math/Vector3Math.h"
#include "application/Provider/IMoveDirectionProvider.h"
#include "engine/base/TakeCFrameWork.h"


BehaviorFloating::BehaviorFloating(IMoveDirectionProvider* provider) {
	moveDirectionProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
	gravity_ = 9.8f; // 重力の強さ
}

void BehaviorFloating::Initialize([[maybe_unused]]GameCharacterContext& characterInfo) {

}

void BehaviorFloating::Update(GameCharacterContext& characterInfo) {
	Vector3& moveDirection_ = characterInfo.moveDirection; // 移動方向
	Vector3& velocity_ = characterInfo.velocity; // 移動ベクトル
	float moveSpeed_ = characterInfo.moveSpeed; // 移動速度
	float deceleration_ = characterInfo.deceleration; // 減速率
	float kMaxMoveSpeed_ = characterInfo.kMaxMoveSpeed; // 最大移動速度

	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		moveDirection_ = Vector3Math::Normalize(moveDirection_);
		//移動時の加速度の計算
		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;
	} else {
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

	// 空中での降下処理(fallSpeedを重力に加算)
	velocity_.y -= (gravity_ + characterInfo.fallSpeed) * deltaTime_;
	characterInfo.transform.translate.x += velocity_.x * deltaTime_;
	characterInfo.transform.translate.z += velocity_.z * deltaTime_;
	characterInfo.transform.translate.y += velocity_.y * deltaTime_;

	// 着地判定
	if (characterInfo.transform.translate.y <= 0.0f) {
		characterInfo.transform.translate.y = 0.0f;
		characterInfo.onGround = true; // 地面に着地したと判断
		nextBehavior_ = Behavior::RUNNING; // 着地したら走行に切り替え
		isTransition_ = true; // 行動の切り替えフラグを立てる
		return;
	}

	// 浮遊中、LTボタンが押された場合STEPBOOSTに切り替え
	// LTボタン＋スティック入力で発動
	//if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::LT)) {
	//	//TriggerStepBoost();
	//}
	//ジャンプボタンの追加入力でさらに上昇
	//TODO: Enemyの浮遊の判断をどうするか
	if( Input::GetInstance()->PushButton(0, GamepadButtonType::RT)) {

		if( characterInfo.overHeatInfo.isOverheated) {
			// オーバーヒート中はジャンプできない
			return;
		}

		// ジャンプのエネルギー消費
		characterInfo.energyInfo.energy -= characterInfo.jumpInfo.useEnergy * deltaTime_;
		// ジャンプの速度を設定
		velocity_.y = characterInfo.jumpInfo.speed;
	}
}

std::pair<bool,Behavior> BehaviorFloating::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return { isTransition_, nextBehavior };
	}

	return { false, Behavior::NONE };
}