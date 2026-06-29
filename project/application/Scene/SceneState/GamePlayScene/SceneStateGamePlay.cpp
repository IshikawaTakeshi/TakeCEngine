#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGamePlay::Initialize(GamePlayScene* scene) {

	// フェーズメッセージUIにFIGHTメッセージをセット
	isFirstUpdate_ = false;
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGamePlay::Update(GamePlayScene* scene) {

	// ================================
	// Manager Update
	// ================================

	TakeCFrameWork::GetUIManager()->Update();
	TakeCFrameWork::GetSpriteManager()->Update();

	// ================================
	// Scene Transition
	// ================================

}
