#include "BehaviorStepBoost.h"
#include "application/Provider/IMoveDirectionProvider.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Quaternion.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

BehaviorStepBoost::BehaviorStepBoost(IMoveDirectionProvider* provider) {
	moveDirectionProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); // デルタタイムの取得
}

void BehaviorStepBoost::Initialize(GameCharacterContext& characterInfo) {
	float speed = characterInfo.stepBoostInfo.speed;
	float duration = characterInfo.stepBoostInfo.duration;
	float useEnergy = characterInfo.stepBoostInfo.useEnergy;


	Vector3 direction = moveDirectionProvider_->GetMoveDirection();

	// オーバーヒート状態のチェック
	if (characterInfo.overHeatInfo.isOverheated) {
		// オーバーヒート中はステップブーストできない
		nextBehavior_ = Behavior::RUNNING;
		isTransition_ = true; // 行動の切り替えフラグを立てる
		return;
	}

	// ステップブーストのエネルギーを消費
	characterInfo.energyInfo.energy -= useEnergy;

	//上昇速度を急激に遅くする
	characterInfo.velocity.y = 0.0f;

	characterInfo.stepBoostInfo.boostTimer = duration;
	characterInfo.velocity.x = direction.x * speed;
	characterInfo.velocity.z = direction.z * speed;
}

void BehaviorStepBoost::Update(GameCharacterContext& characterInfo) {

	float interval = characterInfo.stepBoostInfo.interval;

	// ステップ中の移動
	characterInfo.transform.translate.x += characterInfo.velocity.x * deltaTime_;
	characterInfo.transform.translate.z += characterInfo.velocity.z * deltaTime_;

	characterInfo.stepBoostInfo.boostTimer -= deltaTime_;
	if (characterInfo.stepBoostInfo.boostTimer <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (characterInfo.transform.translate.y <= 0.0f) {
			nextBehavior_ = Behavior::RUNNING;
			isTransition_ = true; // 行動の切り替えフラグを立てる

		} else if (characterInfo.transform.translate.y > 0.0f) {
			nextBehavior_ = Behavior::FLOATING;
			isTransition_ = true; // 行動の切り替えフラグを立てる
		}

		// ステップブーストのインターバルをリセット
		characterInfo.stepBoostInfo.intervalTimer = interval;
	}
}

std::pair<bool, Behavior> BehaviorStepBoost::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return { isTransition_, nextBehavior };
	}

	return { false, Behavior::NONE };
}
