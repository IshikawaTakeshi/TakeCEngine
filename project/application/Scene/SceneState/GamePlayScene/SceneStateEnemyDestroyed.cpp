#include "SceneStateEnemyDestroyed.h"
#include "MyGame.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/camera/CameraManager.h"


//===================================================================================
// 初期化
//===================================================================================
void SceneStateEnemyDestroyed::Initialize(
	[[maybe_unused]] GamePlayScene* scene) {

	// スローモーションにする
	MyGame::RequestTimeScale(-1.0f, 1.0f, 1.0f);
	// カメラをズームする
	TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(
		Camera::GameCameraState::ENEMY_DESTROYED);
	// changeBehaviorTimerを初期化
	changeBehaviorTimer_.Initialize(2.0f, 0.0f);
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateEnemyDestroyed::Update([[maybe_unused]] GamePlayScene* scene) {

	// changeBehaviorTimerの更新
	changeBehaviorTimer_.Update();

	// changeBehaviorTimerが終了したらゲームクリアへ
	if (changeBehaviorTimer_.IsFinished()) {

		// ゲームクリアへ
		RequestTransition(SceneState::GAMECLEAR);

		// ズーム解除
		TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(
			Camera::GameCameraState::FOLLOW);
	}
}
