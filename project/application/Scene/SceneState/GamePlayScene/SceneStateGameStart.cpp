#include "SceneStateGameStart.h"
#include "application/Scene/GamePlayScene.h"

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGameStart::Initialize([[maybe_unused]] GamePlayScene* scene) {

	// フェーズメッセージUIにREADYメッセージをセット
	scene->GetPhaseMessageUI()->SetNextMessage(PhaseMessage::READY);
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGameStart::Update(GamePlayScene* scene) {

	// フェーズメッセージUIが終了したらゲームプレイへ
	if (scene->GetPhaseMessageUI()->GetCurrentMessage() == PhaseMessage::FIGHT) {
		RequestTransition(SceneState::GAMEPLAY);
	}
}
