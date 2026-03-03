#include "SceneStateGameClear.h"
#include "application/MyGame.h"
#include "application/Scene/SceneManager.h"
#include "application/Scene/GamePlayScene.h"


//===================================================================================
// 初期化
//===================================================================================
void SceneStateGameClear::Initialize(GamePlayScene* scene) {

	scene->GetPhaseMessageUI()->SetNextMessage(PhaseMessage::WIN);

	// スローモーション解除
	MyGame::RequestTimeScale(1.0f, 0.6f, 0.0f);
	float fadeTimer = 3.0f;
	SceneManager::GetInstance().ChangeScene("GAMECLEAR", fadeTimer);
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGameClear::Update([[maybe_unused]] GamePlayScene* scene) {}
