#include "PlayerStateIdle.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateRun.h"
#include "PlayerStateJump.h"
#include "PlayerStateAttack.h"

void PlayerStateIdle::Initialize() {}

void PlayerStateIdle::Update() {
	
	//何らかの行動があった時に状態を変更する
	if (Input::GetInstance()->PushKey(DIK_W) ||
		Input::GetInstance()->PushKey(DIK_A) ||
		Input::GetInstance()->PushKey(DIK_S) ||
		Input::GetInstance()->PushKey(DIK_D)) {
		player_->SetState(std::make_unique<PlayerStateRun>(player_));
	} 
	else if (player_->GetIsJumping()) {
		
		player_->SetState(std::make_unique<PlayerStateJump>(player_));
	} 
	else if (Input::GetInstance()->PushKey(DIK_E)) {


		player_->SetState(std::make_unique<PlayerStateAttack>(player_));
	} 
	else {
		//何もしない
	}

}
