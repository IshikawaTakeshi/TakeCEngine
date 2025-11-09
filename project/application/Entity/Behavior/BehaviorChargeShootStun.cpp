#include "BehaviorChargeShootStun.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
BehaviorChargeShootStun::BehaviorChargeShootStun(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

//===================================================================================
//　初期化
//===================================================================================
void BehaviorChargeShootStun::Initialize(PlayableCharacterInfo& characterInfo) {
	// 硬直タイマーの初期化
	characterInfo.chargeAttackStunInfo.stunTimer = characterInfo.chargeAttackStunInfo.stunDuration;
}

//===================================================================================
//　更新
//===================================================================================
void BehaviorChargeShootStun::Update(PlayableCharacterInfo& characterInfo) {
	float& stunTimer_ = characterInfo.chargeAttackStunInfo.stunTimer;
	Vector3& velocity_ = characterInfo.velocity;
	// 硬直時間の更新
	stunTimer_ -= deltaTime_;

	// 速度を大きく落としていく
	velocity_.x *= 0.5f;
	velocity_.z *= 0.5f;

	if (stunTimer_ <= 0.0f) {
		// 状態遷移のフラグを立てる
		if (characterInfo.transform.translate.y <= 0.0f) {
			nextBehavior_ = GameCharacterBehavior::RUNNING;
			isTransition_ = true;

		} else if (characterInfo.transform.translate.y > 0.0f) {
			nextBehavior_ = GameCharacterBehavior::FLOATING;
			isTransition_ = true;
		} else {
			nextBehavior_ = GameCharacterBehavior::RUNNING;
			isTransition_ = true;
			
		}

		characterInfo.velocity = { 0.0f, 0.0f, 0.0f }; // 硬直後の速度をリセット
	}
}
