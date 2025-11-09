#include "BehaviorFloating.h"
#include "engine/io/Input.h"
#include "engine/math/Vector3Math.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
BehaviorFloating::BehaviorFloating(baseInputProvider* provider) {
	inputProvider_ = provider; //入力プロバイダーの設定
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); //デルタタイムの取得
	gravity_ = 9.8f; //重力の強さ
}

//===================================================================================
//　初期化
//===================================================================================
void BehaviorFloating::Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) {
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); //デルタタイムの取得
}

//===================================================================================
//　更新
//===================================================================================
void BehaviorFloating::Update(PlayableCharacterInfo& characterInfo) {

	Vector3& moveDirection_ = characterInfo.moveDirection; // 移動方向
	Vector3& velocity_ = characterInfo.velocity; // 移動ベクトル
	float moveSpeed_ = characterInfo.moveSpeed; // 移動速度
	float deceleration_ = characterInfo.deceleration; // 減速率
	float kMaxMoveSpeed_ = characterInfo.kMaxMoveSpeed; // 最大移動速度

	// コントローラー入力時、速度の更新
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
	characterInfo.transform.translate += velocity_ * deltaTime_;
	
	// 地面に着地したらRUNNINGに戻る
	if(characterInfo.onGround == true) {
		isTransition_ = true;
		nextBehavior_ = GameCharacterBehavior::RUNNING;
		return;
	}

	//ジャンプボタンの追加入力でさらに上昇
	if(inputProvider_->IsJumpRequested()) {

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