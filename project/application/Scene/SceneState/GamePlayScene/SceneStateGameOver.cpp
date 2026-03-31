#include "SceneStateGameOver.h"
#include "application/Scene/SceneManager.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGameOver::Initialize(GamePlayScene* scene) {

	scene->GetPhaseMessageUI()->SetNextMessage(PhaseMessage::LOSE);

	float fadeTimer = 3.0f;
	SceneManager::GetInstance().ChangeScene("GAMEOVER", fadeTimer);
	//TakeCFrameWork::GetPostEffectManager()->PlayEffect("GameOverEffect_1");
	TakeCFrameWork::GetPostEffectManager()->PlayEffect("GameOverEffect_3");
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGameOver::Update([[maybe_unused]] GamePlayScene* scene) {}
