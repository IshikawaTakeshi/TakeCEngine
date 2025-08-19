#include "BehaviorChargeShoot.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

BehaviorChargeShoot::BehaviorChargeShoot(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void BehaviorChargeShoot::Initialize(GameCharacterContext& characterInfo) {
	characterInfo;
}

void BehaviorChargeShoot::Update(GameCharacterContext& characterInfo) {


	Vector3& velocity_ = characterInfo.velocity; // 移動ベクトル

	// 速度を大きく落としていき、停止させた後攻撃処理を入れる
	velocity_.x *= 0.5f;
	velocity_.z *= 0.5f;
	if (std::abs(velocity_.x) < 0.01f && std::abs(velocity_.z) < 0.01f) {
		// チャージ攻撃の実行
		inputProvider_->RequestChargeAttack();

		//if (weapon->IsMoveShootable()) {
		//	// ステップ終了時前回の状態に戻す
		//	if (characterInfo_.transform.translate.y <= 0.0f) {
		//		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
		//	} else if (characterInfo_.transform.translate.y > 0.0f) {
		//		behaviorManager_->RequestBehavior(GameCharacterBehavior::FLOATING);
		//	} else {
		//		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
		//	}
		//} else {
		//	// 停止撃ち専用の場合はCHARGESHOOT_STUNに切り替え
		//	behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
		//}

	}
	

	// 位置の更新（deltaTimeをここで適用）
	characterInfo.transform.translate.x += velocity_.x * deltaTime_;
	characterInfo.transform.translate.z += velocity_.z * deltaTime_;
}
