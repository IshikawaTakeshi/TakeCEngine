#include "BehaviorChargeShootStun.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

BehaviorChargeShootStun::BehaviorChargeShootStun(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void BehaviorChargeShootStun::Initialize(GameCharacterContext& characterInfo) {
	// 硬直タイマーの初期化
	characterInfo.chargeAttackStunInfo.stunTimer = characterInfo.chargeAttackStunInfo.stunDuration;
}

void BehaviorChargeShootStun::Update(GameCharacterContext& characterInfo) {
	float& stunTimer_ = characterInfo.chargeAttackStunInfo.stunTimer;

	stunTimer_ -= deltaTime_;
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
	}
}
