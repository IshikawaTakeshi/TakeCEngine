#include "Player.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"
#include "Input.h"

void Player::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//キャラクタータイプ設定
	characterType_ = CharacterType::PLAYER;
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
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

}

void Player::Draw() {}

void Player::InitRunning() {}

void Player::InitJump() {}

void Player::InitDash() {}

void Player::UpdateRunning() {

	//左スティック
	StickState leftJoystickState = Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightJoystickState = Input::GetInstance()->GetRightStickState(0);
	//左スティックの移動量を取得
	float leftX = leftJoystickState.x;
	float leftY = leftJoystickState.y;
	
	//移動処理
	object3d_->SetTranslate({
		object3d_->GetTranslate().x + leftX * 0.1f,
		object3d_->GetTranslate().y,
		object3d_->GetTranslate().z + leftY * 0.1f
		});
}

void Player::UpdateAttack() {}

void Player::UpdateDamage() {}

void Player::UpdateJump() {}

void Player::UpdateDash() {}
