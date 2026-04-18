#include "GamePlayScene.h"
#include "Collision/CollisionManager.h"
#include "ImGuiManager.h"
#include "MyGame.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "Vector3Math.h"
#include <format>
#include <numbers>

// SceneState includes
#include "application/Scene/SceneState/GamePlayScene/SceneStateEnemyDestroyed.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameClear.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameOver.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGamePlay.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameStart.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStatePause.h"


//====================================================================
//			初期化
//====================================================================

void GamePlayScene::Initialize() {

	// BGM読み込み
	BGM_ = AudioManager::GetInstance().LoadSound("GamePlaySceneBGM.mp3");
	bgmVolume_ = 0.2f;

	// Camera0
	gameCamera_ = std::make_unique<Camera>();
	gameCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_GameScene.json");
	gameCamera_->RequestCameraState(Camera::GameCameraState::FOLLOW);
	TakeC::CameraManager::GetInstance().AddCamera("gameCamera", gameCamera_.get());

	// Camera1
	debugCamera_ = std::make_unique<Camera>();
	debugCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_GameScene.json");
	TakeC::CameraManager::GetInstance().AddCamera("debugCamera", debugCamera_.get());

	// lightCamera
	lightCamera_ = std::make_unique<Camera>();
	lightCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_LightCamera.json");
	lightCamera_->SetProjectionChanged(true);
	TakeC::CameraManager::GetInstance().AddCamera("lightCamera", lightCamera_.get());

	// デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(
		TakeC::CameraManager::GetInstance().GetActiveCamera());
	ParticleCommon::GetInstance().SetDefaultCamera(
		TakeC::CameraManager::GetInstance().GetActiveCamera());

	// SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(),
		"skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f, 0.2f, 0.2f, 1.0f });

	//ポーズメニューUI
	TakeCFrameWork::GetUIManager()->CreateUI<PauseMenuUI>("PauseMenuUI");
	TakeCFrameWork::GetUIManager()->SetUIActive("PauseMenuUI", false); // 最初は非表示

	// シーンステートの初期化
	sceneStateManager_.RegisterState(SceneState::GAMESTART,
		std::make_unique<SceneStateGameStart>());
	sceneStateManager_.RegisterState(SceneState::GAMEPLAY,
		std::make_unique<SceneStateGamePlay>());
	sceneStateManager_.RegisterState(SceneState::GAMEOVER,
		std::make_unique<SceneStateGameOver>());
	sceneStateManager_.RegisterState(SceneState::GAMECLEAR,
		std::make_unique<SceneStateGameClear>());
	sceneStateManager_.RegisterState(SceneState::PAUSE,
		std::make_unique<SceneStatePause>());
	sceneStateManager_.Initialize(SceneState::GAMESTART, this);

	// ShadowMapEffectを有効化
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect",true);
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("DepthBasedOutline", true);
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {

	AudioManager::GetInstance().SoundUnload(&BGM_);         // BGMの解放
	CollisionManager::GetInstance().ClearGameCharacter();   // 当たり判定の解放
	TakeC::CameraManager::GetInstance().ResetCameras();     // カメラのリセット
	TakeCFrameWork::GetParticleManager()->ClearParticles(); // パーティクルの解放
	TakeCFrameWork::GetParticleManager()->ClearEmitters();  // エミッターの解放
	TakeCFrameWork::GetLightManager()->ClearAllPointLights();  // ポイントライトの解放
	TakeCFrameWork::GetSpriteManager()->Clear(); // スプライトの解放
	TakeCFrameWork::GetUIManager()->Clear();     // UIの解放
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void GamePlayScene::Update() {

	// BGM再生
	if (!isSoundPlay_) {
		AudioManager::GetInstance().SoundPlayWave(BGM_, bgmVolume_, true);
		isSoundPlay_ = true;
	}

	// SkyBoxの更新
	skyBox_->Update();
	// シーンステートの更新
	sceneStateManager_.Update(this);

	if(isPauseMenuActive_){
		return; // 以降の更新処理をスキップ
	}

	// カメラの更新
	TakeC::CameraManager::GetInstance().Update();
	// particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();
	// LightManager更新
	/*Camera* lightCam =
		TakeC::CameraManager::GetInstance().FindCameraByName("lightCamera");
	lightCam->Update();
	TakeCFrameWork::GetLightManager()->UpdateShadowMatrix(
		lightCam, player_->GetObject3d()->GetWorldPosition());*/

	// 当たり判定の更新
	CheckAllCollisions();
}

void GamePlayScene::UpdateImGui() {

	TakeC::CameraManager::GetInstance().UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();
	TakeCFrameWork::GetSpriteManager()->UpdateImGui();
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw(); // 天球の描画

#pragma region 背景スプライト描画
	// スプライトの描画前処理
	SpriteCommon::GetInstance().PreDraw();

#pragma endregion

#pragma region Object3d描画

	// Object3dの描画前Dispatch処理
	Object3dCommon::GetInstance().Dispatch();

	// Object3dの描画前処理
	Object3dCommon::GetInstance().PreDraw();

#pragma endregion
  // spotLightの描画
	TakeCFrameWork::GetLightManager()->DrawSpotLights();
	// 登録されたワイヤーフレームをすべて描画させる
	TakeCFrameWork::GetWireFrame()->Draw();
	// パーティクルの描画
	TakeCFrameWork::GetParticleManager()->Draw();
}

void GamePlayScene::DrawSprite() {

	// スプライトの描画前処理
	SpriteCommon::GetInstance().PreDraw();

	if (sceneStateManager_.GetCurrentStateType() != SceneState::ENEMYDESTROYED) {

		TakeCFrameWork::GetSpriteManager()->Draw();
	}
}

//====================================================================
//			影描画処理
//====================================================================
void GamePlayScene::DrawShadow() {

	// ライトカメラのセット
	//const LightCameraInfo& lightCameraInfo = TakeCFrameWork::GetLightManager()->GetLightCameraInfo();
	// Object3dの影描画前処理
	Object3dCommon::GetInstance().PreDrawShadowPass();
}

//====================================================================
//			全ての当たり判定のチェック
//====================================================================
void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance().ClearGameCharacter();

	//const std::vector<Bullet*>& playerBullets =
	//	bulletManager_->GetAllPlayerBullets();


	//// プレイヤーの登録
	//CollisionManager::GetInstance().RegisterGameCharacter(
	//	static_cast<GameCharacter*>(player_.get()));
	//// 敵キャラクターの登録
	//CollisionManager::GetInstance().RegisterGameCharacter(
	//	static_cast<GameCharacter*>(enemy_.get()));
	//// BulletSensorの登録
	//CollisionManager::GetInstance().RegisterGameCharacter(
	//	static_cast<GameCharacter*>(enemy_->GetBulletSensor()));

	//// 弾の登録
	//for (const auto& bullet : playerBullets) {
	//	if (bullet->IsActive()) {
	//		CollisionManager::GetInstance().RegisterGameCharacter(
	//			static_cast<GameCharacter*>(bullet));
	//	}
	//}


	CollisionManager::GetInstance().CheckAllCollisionsForGameCharacter();
}