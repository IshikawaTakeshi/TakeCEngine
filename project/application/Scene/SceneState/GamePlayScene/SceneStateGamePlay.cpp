#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGamePlay::Initialize([[maybe_unused]] GamePlayScene* scene) {

	/*TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(
		Camera::GameCameraState::LOCKON);*/


	isFirstUpdate_ = false;
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGamePlay::Update([[maybe_unused]] GamePlayScene* scene) {
	

	// ================================
	// Manager Update
	// ================================

	TakeCFrameWork::GetUIManager()->Update();
	TakeCFrameWork::GetSpriteManager()->Update();

	// ================================
	// Scene Transition
	// ================================

	if(Input::GetInstance().TriggerButton(0,GamepadButtonType::Start)) {
		//ポストエフェクトを再生
		TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeOut");
		RequestTransition(SceneState::PAUSE);
	}
}
