#include "PlayerStateRun.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateIdle.h"
#include "PlayerStateJump.h"
#include "PlayerStateAttack.h"

void PlayerStateRun::Initialize() {}

void PlayerStateRun::Update() {
	//const float moveSpeed = 0.5f;

	// ゲームパッドの状態を取得
	//if (Input::GetInstance()->PushKey(DIK_A)) {
	//	player_->SetVelocityX(player_->GetVelocity().x - moveSpeed);
	//} else if (Input::GetInstance()->PushKey(DIK_D)) {
	//	player_->SetVelocityX(player_->GetVelocity().x + moveSpeed);
	//}

	//if (Input::GetInstance()->PushKey(DIK_W)) {
	//	player_->SetVelocityZ(player_->GetVelocity().z + moveSpeed);
	//} else if (Input::GetInstance()->PushKey(DIK_S)) {
	//	player_->SetVelocityZ(player_->GetVelocity().z - moveSpeed);
	//}

	//// 座標移動(ベクトルの加算)
	//player_->SetWorldPos(player_->GetWorldPos() + player_->GetVelocity());

	//入力がないときはIdle状態に遷移
	if (!Input::GetInstance()->PushKey(DIK_W) &&
		!Input::GetInstance()->PushKey(DIK_A) &&
		!Input::GetInstance()->PushKey(DIK_S) &&
		!Input::GetInstance()->PushKey(DIK_D) &&
		player_->GetIsJumping() == false) {
		player_->SetState(std::make_unique<PlayerStateIdle>(player_));
	}
	//SPACEキーでジャンプ
	else if (player_->GetIsJumping()) {
		player_->SetState(std::make_unique<PlayerStateJump>(player_));
	}
	//攻撃処理が入力されたらAttack状態に遷移
	else if (Input::GetInstance()->PushKey(DIK_E)) {
		player_->SetState(std::make_unique<PlayerStateAttack>(player_));
	} else {
		//何もしない
	}
}
