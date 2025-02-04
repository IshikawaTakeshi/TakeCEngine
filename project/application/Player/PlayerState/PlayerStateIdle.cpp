#include "PlayerStateIdle.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateRun.h"
#include "PlayerStateJump.h"
#include "PlayerStateAttack.h"
#include "TakeCFrameWork.h"

void PlayerStateIdle::Initialize() {
	
	//待機アニメーションの設定
	stateAnimation_ = TakeCFrameWork::GetAnimator()->FindAnimation("Idle.gltf");
	player_->GetModel()->SetAnimation(stateAnimation_);
}

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
	else if (Input::GetInstance()->PushKey(DIK_K)) {


		//player_->SetState(std::make_unique<PlayerStateAttack>(player_));
	} 
	else {
		//何もしない
	}

}
