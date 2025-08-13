#include "BehaviorJumping.h"
#include "engine/base/TakeCFrameWork.h"

void BehaviorJumping::Initialize(GameCharacterContext& characterInfo) {


	deltaTime_ = TakeCFrameWork::GetDeltaTime(); // デルタタイムの取得
	
	//オーバーヒート状態のチェック
	if (characterInfo.overHeatInfo.isOverheated) {
		// オーバーヒート中はジャンプできない
		TransitionNextBehavior(Behavior::RUNNING);
		return;
	}

	// ジャンプのエネルギー消費
	characterInfo.energyInfo.energy -= characterInfo.jumpInfo.useEnergy;

	//ジャンプの初期化
	//ジャンプの速度を設定
	characterInfo.velocity.y = characterInfo.jumpInfo.speed;

	//ジャンプ中の移動方向を設定
	//StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
	//Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	//Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	//characterInfo_.moveDirection = forward * leftStick.y + right * leftStick.x;

	characterInfo.transform.translate.y += 0.1f; // 少し上に移動してジャンプ感を出す
}

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
		TransitionNextBehavior(Behavior::FLOATING);
		return;
	}

	// 地面に着地したらRUNNINGに戻る
	if (characterInfo.transform.translate.y <= 0.0f) {
		characterInfo.transform.translate.y = 0.0f; // 地面に合わせる
		TransitionNextBehavior(Behavior::RUNNING);
		velocity = { 0.0f, 0.0f, 0.0f }; // ジャンプ中の速度をリセット
	}
}

std::optional<GameCharacterBehavior> BehaviorJumping::TransitionNextBehavior(GameCharacterBehavior nextBehavior) {
	if (nextBehavior != GameCharacterBehavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return nextBehavior;
	}
	return std::nullopt; // 次の行動がない場合はnulloptを返す
}
