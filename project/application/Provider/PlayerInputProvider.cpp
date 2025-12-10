#include "PlayerInputProvider.h"
#include "engine/Input/Input.h"
#include "application/Entity/Player/Player.h"
#include "application/Provider/PlayerKeyInput.h"
#include "application/Provider/PlayerPadInput.h"

PlayerInputProvider::PlayerInputProvider(Player* player) {
	mapper_ = std::make_unique<InputMapper<CharacterActionInput>>();
	//mapper_->AddDevice(std::make_unique<PlayerKeyInput>());
	mapper_->AddDevice(std::make_unique<PlayerPadInput>());
	player_ = player;
}

//=====================================================================================
//　移動方向の取得
//=====================================================================================
Vector3 PlayerInputProvider::GetMoveDirection() const {
	Vector2 move = mapper_->GetVector2(CharacterActionInput::MOVE);
	// カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), player_->GetCamera()->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1.0f, 0.0f, 0.0f), player_->GetCamera()->GetRotate());
	return forward * move.y + right * move.x;
}

Vector2 PlayerInputProvider::GetCameraRotateInput() const {
	Vector2 cameraMove = mapper_->GetVector2(CharacterActionInput::MOVE_CAMERA);
	return cameraMove;
}

//=====================================================================================
//　ジャンプ入力の取得
//=====================================================================================
bool PlayerInputProvider::RequestJumpInput() const {
	return mapper_->IsPressed(CharacterActionInput::JUMP);
}

//=====================================================================================
//　攻撃入力の取得
//=====================================================================================
bool PlayerInputProvider::RequestAttack(CharacterActionInput attackButton) {
	return mapper_->IsPressed(attackButton);
}

//=====================================================================================
//　攻撃入力を離したか取得

bool PlayerInputProvider::ReleaseAttackInput(CharacterActionInput attackButton) {
	return mapper_->IsReleased(attackButton);
}

//=====================================================================================
//　チャージ攻撃入力の取得
//=====================================================================================
bool PlayerInputProvider::RequestChargeAttack(CharacterActionInput attackButton) {
	return mapper_->IsPressed(attackButton);
}

//=====================================================================================
//　ステップブースト入力の取得
//=====================================================================================
bool PlayerInputProvider::RequestStepBoost() const {
	return mapper_->IsPressed(CharacterActionInput::STEPBOOST);
}

bool PlayerInputProvider::RequestChangeCameraMode() const {
	return mapper_->IsTriggered(CharacterActionInput::LOCKON);
}
