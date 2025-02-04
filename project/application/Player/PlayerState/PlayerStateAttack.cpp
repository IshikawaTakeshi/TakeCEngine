#include "PlayerStateAttack.h"
#include "Player/Player.h"
#include "Input.h"
#include "PlayerStateIdle.h"
#include "PlayerStateJump.h"
#include "PlayerStateRun.h"
#include "TakeCFrameWork.h"



void PlayerStateAttack::Initialize() {

	//Attackアニメーションの設定
	stateAnimation_ = TakeCFrameWork::GetAnimator()->FindAnimation("throwAttack.gltf");
	player_->GetModel()->SetAnimation(stateAnimation_);
}

void PlayerStateAttack::Update() {

	////Animation終了時にIdle状態に遷移
	//float animationTime = player_->GetModel()->GetAnimationTime();
	//float duration = player_->GetModel()->GetAnimation().duration;
	//if (animationTime > duration) {
	//	player_->SetState(std::make_unique<PlayerStateIdle>(player_));
	//} 
}
