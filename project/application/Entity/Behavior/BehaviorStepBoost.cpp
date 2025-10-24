#include "BehaviorStepBoost.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
BehaviorStepBoost::BehaviorStepBoost(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); // デルタタイムの取得
}

//===================================================================================
//　初期化
//===================================================================================
void BehaviorStepBoost::Initialize(GameCharacterContext& characterInfo) {

	float speed = characterInfo.stepBoostInfo.speed;         // ステップブーストの速度
	float duration = characterInfo.stepBoostInfo.duration;   // ステップブーストの持続時間
	float useEnergy = characterInfo.stepBoostInfo.useEnergy; // ステップブーストに必要なエネルギー

	// 入力方向の取得
	Vector3 direction = inputProvider_->GetMoveDirection();

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

//===================================================================================
//　更新
//===================================================================================
void BehaviorStepBoost::Update(GameCharacterContext& characterInfo) {

	// インターバル時間の取得
	float interval = characterInfo.stepBoostInfo.interval;

	// ステップ中の移動
	characterInfo.transform.translate.x += characterInfo.velocity.x * deltaTime_;
	characterInfo.transform.translate.z += characterInfo.velocity.z * deltaTime_;
	characterInfo.stepBoostInfo.boostTimer -= deltaTime_;

	// ステップブーストの持続時間が終了したら元の状態に戻す
	if (characterInfo.stepBoostInfo.boostTimer <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (characterInfo.onGround == true) {
			nextBehavior_ = Behavior::RUNNING;
			isTransition_ = true; // RUNNINGに切り替え

		} else {
			nextBehavior_ = Behavior::FLOATING;
			isTransition_ = true; // FLOATINGに切り替え
		}

		// ステップブーストのインターバルをリセット
		characterInfo.stepBoostInfo.intervalTimer = interval;
	}
}