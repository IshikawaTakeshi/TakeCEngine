#include "SceneStateGameClear.h"
#include "application/MyGame.h"
#include "application/Scene/SceneManager.h"
#include "application/Scene/GamePlayScene.h"


//===================================================================================
// 初期化
//===================================================================================
void SceneStateGameClear::Initialize([[maybe_unused]] GamePlayScene* scene) {
	// スローモーション解除
	MyGame::RequestTimeScale(1.0f, 0.4f, 0.0f);
	float fadeTimer = 1.0f;
	TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeOut");
	SceneManager::GetInstance().ChangeScene("GAMECLEAR", fadeTimer);
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGameClear::Update([[maybe_unused]] GamePlayScene* scene) {

}
