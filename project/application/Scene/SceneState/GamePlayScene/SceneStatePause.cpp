#include "SceneStatePause.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/Base/TakeCFrameWork.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStatePause::Initialize([[maybe_unused]] GamePlayScene *scene) {

	// ポーズメニューUIを開く
	TakeCFrameWork::GetUIManager()->GetUI<PauseMenuUI>("PauseMenuUI")->Open();
	// ポーズフラグを立てる
	scene->SetPauseMenuActive(true);

}

//===================================================================================
// 更新
//===================================================================================
void SceneStatePause::Update([[maybe_unused]] GamePlayScene *scene) {

	TakeCFrameWork::GetUIManager()->Update();
	TakeCFrameWork::GetSpriteManager()->Update();

	if(Input::GetInstance().TriggerButton(0,GamepadButtonType::Start)) {

		// ポーズメニューUIを閉じる
		TakeCFrameWork::GetUIManager()->GetUI<PauseMenuUI>("PauseMenuUI")->Close();
		// ポーズフラグを下ろす
		scene->SetPauseMenuActive(false);
		// ゲームプレイ状態に遷移
		RequestTransition(SceneState::GAMEPLAY);
	}
}
