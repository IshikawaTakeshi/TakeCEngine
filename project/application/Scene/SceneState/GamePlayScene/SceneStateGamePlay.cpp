#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGamePlay::Initialize([[maybe_unused]] GamePlayScene* scene) {

	// フェーズメッセージUIにFIGHTメッセージをセット
	isFirstUpdate_ = false;
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGamePlay::Update([[maybe_unused]] GamePlayScene* scene) {

	// ================================
	// Manager Update
	// ================================

	TakeC::TakeCFrameWork::GetUIManager()->Update();
	TakeC::TakeCFrameWork::GetSpriteManager()->Update();

	// ================================
	// Scene Transition
	// ================================

}
