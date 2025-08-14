#include "BehaviorJumping.h"
#include "engine/base/TakeCFrameWork.h"
#include "application/Provider/IMoveDirectionProvider.h"

BehaviorJumping::BehaviorJumping(IMoveDirectionProvider* provider) {
	moveDirectionProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); // デルタタイムの取得
	
}

//=========================================================================
// 初期化処理
//=========================================================================

void BehaviorJumping::Initialize(GameCharacterContext& characterInfo) {
	// ジャンプのエネルギー消費
	characterInfo.energyInfo.energy -= characterInfo.jumpInfo.useEnergy;
	//ジャンプの速度を設定
	characterInfo.velocity.y = characterInfo.jumpInfo.speed;
	// 少し上に移動してジャンプ感を出す
	characterInfo.transform.translate.y += 0.1f; 
}

//=========================================================================
// 更新処理
//=========================================================================

void BehaviorJumping::Update(GameCharacterContext& characterInfo) {

	Vector3& velocity = characterInfo.velocity; // 移動ベクトル

	// ジャンプ中の移動
	characterInfo.transform.translate.x += velocity.x * deltaTime_;
	characterInfo.transform.translate.z += velocity.z * deltaTime_;
	// 重力の適用
	velocity.y -= (gravity_ + characterInfo.jumpInfo.deceleration) * deltaTime_;
	characterInfo.transform.translate.y += velocity.y * deltaTime_;

	characterInfo.jumpInfo.jumpTimer += deltaTime_;
	if (characterInfo.jumpInfo.jumpTimer > characterInfo.jumpInfo.maxJumpTime) {

		characterInfo.jumpInfo.jumpTimer = 0.0f; // ジャンプタイマーをリセット
		//FLOATINGに切り替え
		isTransition_ = true;
		nextBehavior_ = Behavior::FLOATING;
		return;
	}

	// 地面に着地したらRUNNINGに戻る
	if (characterInfo.transform.translate.y <= 0.0f) {
		characterInfo.transform.translate.y = 0.0f; // 地面に合わせる
		isTransition_ = true;
		nextBehavior_ = Behavior::RUNNING;
		velocity = { 0.0f, 0.0f, 0.0f }; // ジャンプ中の速度をリセット
	}
}

std::pair<bool,Behavior> BehaviorJumping::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return { isTransition_, nextBehavior };
	}

	return { false, Behavior::NONE };
}
