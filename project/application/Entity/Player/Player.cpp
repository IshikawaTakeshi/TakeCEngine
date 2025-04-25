#include "Player.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"
#include "Input.h"
#include "camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"

void Player::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//キャラクタータイプ設定
	characterType_ = CharacterType::PLAYER;

	behaviorRequest_ = Behavior::RUNNING;
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());

	camera_ = CameraManager::GetInstance()->GetActiveCamera();
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void Player::Update() {

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case Behavior::IDLE:
			break;
		case Behavior::RUNNING:
			InitRunning();
			break;
		case Behavior::JUMP:
			InitJump();
			break;
		case Behavior::DASH:
			InitDash();
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case Player::Behavior::IDLE:
		break;
	case Player::Behavior::RUNNING:
		UpdateRunning();
		break;
	case Player::Behavior::JUMP:
		UpdateJump();
		break;
	case Player::Behavior::DASH:
		UpdateDash();
		break;
	default:
		break;
	}

	//カメラの設定
	camera_->SetTargetPos(transform_.translate);
	camera_->SetTargetRot(transform_.rotate);

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetRotation(transform_.rotate);
	object3d_->Update();
	collider_->Update(object3d_.get());
}

void Player::Draw() {
	object3d_->Draw();
}

void Player::DrawCollider() {
#ifdef _DEBUG
	collider_->DrawCollider();
#endif
}

void Player::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::ENEMY) {
		//衝突時の処理
	}
}

void Player::InitRunning() {}

void Player::InitJump() {}

void Player::InitDash() {}

void Player::UpdateRunning() {

	//左スティック
	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	//カメラの回転を設定
	camera_->SetStick({ rightStick.x, rightStick.y });

	//カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0, 0, 1), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	moveDirection_ = forward * leftStick.y + right * leftStick.x;

	//移動時の加速度の計算
	velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
	velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;
	if (velocity_.x > kMaxMoveSpeed_) {
		velocity_.x = kMaxMoveSpeed_;
	}
	if (velocity_.z > kMaxMoveSpeed_) {
		velocity_.z = kMaxMoveSpeed_;
	}

	//速度の減速処理
	velocity_.x /= deceleration_;
	velocity_.z /= deceleration_;

	//移動処理
	transform_.translate = {
		transform_.translate.x + velocity_.x * deltaTime_,
		transform_.translate.y,
		transform_.translate.z + velocity_.z * deltaTime_
	};
}

void Player::UpdateAttack() {}

void Player::UpdateDamage() {}

void Player::UpdateJump() {}

void Player::UpdateDash() {}
