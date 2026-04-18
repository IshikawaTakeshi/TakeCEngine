#include "SceneStatePause.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/Base/TakeCFrameWork.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStatePause::Initialize([[maybe_unused]] GamePlayScene* scene) {

	// ポーズメニューUIを開く
	TakeCFrameWork::GetUIManager()->GetUI<PauseMenuUI>("PauseMenuUI")->Open();
	// ポーズフラグを立てる
	scene->SetPauseMenuActive(true);

}

//===================================================================================
// 更新
//===================================================================================
void SceneStatePause::Update([[maybe_unused]] GamePlayScene* scene) {

	TakeCFrameWork::GetUIManager()->Update();
	TakeCFrameWork::GetSpriteManager()->Update();

	PauseMenuUI* pauseMenuUI = TakeCFrameWork::GetUIManager()->GetUI<PauseMenuUI>("PauseMenuUI");

	//pauseMenuUIの結果を受け取る
	switch (pauseMenuUI->ConsumeResult()) {
	case PauseMenuResult::Resume:
	{
		// ポーズフラグを下ろす
		scene->SetPauseMenuActive(false);
		// ゲームプレイ状態に遷移
		RequestTransition(SceneState::GAMEPLAY);
		break;
	}
	case PauseMenuResult::Retry:
	{
		// ポーズフラグを下ろす
		scene->SetPauseMenuActive(false);
		// ゲーム開始状態に遷移
		float fadeTimer = 1.0f;
		TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeOut");
		SceneManager::GetInstance().ChangeScene("GAMEPLAY", fadeTimer);
		break;
	}
	case PauseMenuResult::ToTitle:
	{
		// ポーズフラグを下ろす
		scene->SetPauseMenuActive(false);
		// ゲーム開始状態に遷移
		float fadeTimer = 1.0f;
		TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeOut");
		SceneManager::GetInstance().ChangeScene("TITLE", fadeTimer);
		break;
	}
	default:
		break;
	}
}
