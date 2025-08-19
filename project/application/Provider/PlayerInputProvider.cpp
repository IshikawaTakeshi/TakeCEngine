#include "PlayerInputProvider.h"
#include "engine/io/Input.h"
#include "application/Entity/Player/Player.h"

Vector3 PlayerInputProvider::GetMoveDirection() const {
	//左スティック
	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	//カメラのスティック入力をカメラに反映
	player_->GetCamera()->SetStick({ rightStick.x, rightStick.y });
	
	//カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), player_->GetCamera()->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), player_->GetCamera()->GetRotate());
	return forward * leftStick.y + right * leftStick.x;
}

void PlayerInputProvider::RequestAttack() {
	for(auto& weapon : player_->GetWeapons()) {
		if (weapon->IsChargeAttack()) {
			weapon->ChargeAttack();
		} else {
			// チャージ攻撃不可の場合は通常攻撃
			weapon->Attack();
		}
	}
}

void PlayerInputProvider::RequestChargeAttack() {
	for(auto& weapon : player_->GetWeapons()) {
		weapon->Attack();
	}
}
