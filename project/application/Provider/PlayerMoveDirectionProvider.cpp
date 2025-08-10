#include "PlayerMoveDirectionProvider.h"
#include "engine/io/Input.h"
#include "application/Entity/Player/Player.h"

Vector3 PlayerMoveDirectionProvider::GetMoveDirection() const {
	//左スティック
	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	
	//カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), player_->GetCamera()->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), player_->GetCamera()->GetRotate());
	return forward * leftStick.y + right * leftStick.x;
}
