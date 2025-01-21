#include "PlayerStateAttack.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateIdle.h"
#include "PlayerStateJump.h"

void PlayerStateAttack::Initialize() {}

void PlayerStateAttack::Update() {

	//攻撃入力がないときはIdle状態に遷移
	if (!Input::GetInstance()->PushKey(DIK_E)) {
		player_->SetState(std::make_unique<PlayerStateIdle>(player_));
	}
	//ジャンプしているならJump状態に遷移
	else if (player_->GetIsJumping()) {
		player_->SetState(std::make_unique<PlayerStateJump>(player_));
	}
}
