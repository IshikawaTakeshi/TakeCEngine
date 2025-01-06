#include "PlayerStateJump.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateIdle.h"


void PlayerStateJump::Initialize() {

}

void PlayerStateJump::Update() {

	//ジャンプ処理
	if (player_->GetIsJumping()) {
		//jumpVelocity_ += gravity_; // 重力を加算して下降
		player_->SetJumpVelocity(player_->GetJumpVelocity() + player_->GetGravity());
		//transform_.translate.y += jumpVelocity_;
		player_->SetWorldPosY(player_->GetWorldPos().y + player_->GetJumpVelocity());

		// 地面に着地した場合
		if (player_->GetWorldPos().y <= player_->GetPlayerFloor()) {
			player_->SetWorldPosY(player_->GetPlayerFloor());
			player_->SetIsJumping(false);
			player_->SetJumpVelocity(0.0f);
		}
	} else {
		player_->SetState(std::make_unique<PlayerStateIdle>(player_));
	}
}
