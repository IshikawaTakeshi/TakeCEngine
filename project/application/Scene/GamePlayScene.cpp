#include "GamePlayScene.h"
#include <format>
#include <numbers>
#include "application/MyGame.h"
#include "application/Scene/SceneManager.h"
#include "application/Scene/TitleScene.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Collision/CollisionManager.h"
#include "engine/Math/Vector3Math.h"

// SceneState includes
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameClear.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameOver.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGamePlay.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameStart.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStatePause.h"

using namespace TakeC;

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

	
	// シーンステートの初期化
	sceneStateManager_.RegisterState(SceneState::GAMEPLAY,
		std::make_unique<SceneStateGamePlay>());
	sceneStateManager_.Initialize(SceneState::GAMEPLAY, this);

	// ShadowMapEffectを有効化
	TakeC::TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect",true);
	TakeC::TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeIn");
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {

	AudioManager::GetInstance().SoundUnload(&BGM_);         // BGMの解放
	TakeC::TakeCFrameWork::GetCameraCapture()->Finalize();             // カメラキャプチャの解放
	CollisionManager::GetInstance().ClearGameCharacter();   // 当たり判定の解放
	TakeC::CameraManager::GetInstance().ResetCameras();     // カメラのリセット
	TakeC::TakeCFrameWork::GetParticleManager()->ClearParticles(); // パーティクルの解放
	TakeC::TakeCFrameWork::GetParticleManager()->ClearEmitters();  // エミッターの解放
	TakeC::TakeCFrameWork::GetLightManager()->ClearAllPointLights();  // ポイントライトの解放
	TakeC::TakeCFrameWork::GetPostEffectManager()->StopAllEffects();    // ポストエフェクトの停止
	TakeC::TakeCFrameWork::GetSpriteManager()->Clear(); // スプライトの解放
	TakeC::TakeCFrameWork::GetUIManager()->Clear();     // UIの解放
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

	//CameraCaptureの更新
	TakeC::TakeCFrameWork::GetCameraCapture()->Update();

	// SkyBoxの更新
	skyBox_->Update();
	// シーンステートの更新
	sceneStateManager_.Update(this);

	// カメラの更新
	TakeC::CameraManager::GetInstance().Update();

	// particleManager更新
	TakeC::TakeCFrameWork::GetParticleManager()->Update();
	// LightManager更新
	/*Camera* lightCam =
		TakeC::CameraManager::GetInstance().FindCameraByName("lightCamera");
	lightCam->Update();
	TakeC::TakeCFrameWork::GetLightManager()->UpdateShadowMatrix(
		lightCam, player_->GetObject3d()->GetWorldPosition());*/

	// 当たり判定の更新
	CheckAllCollisions();
}

void GamePlayScene::UpdateImGui() {

	TakeC::CameraManager::GetInstance().UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();
	TakeC::TakeCFrameWork::GetSpriteManager()->UpdateImGui();

	
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw(); // 天球の描画

#pragma region Object3d描画

	// Object3dの描画前Dispatch処理
	Object3dCommon::GetInstance().Dispatch();
	
	// Object3dの描画前処理
	Object3dCommon::GetInstance().PreDraw();
	
#pragma endregion

  // spotLightの描画
	TakeC::TakeCFrameWork::GetLightManager()->DrawSpotLights();
	// 登録されたワイヤーフレームをすべて描画させる
	TakeC::TakeCFrameWork::GetWireFrame()->Draw();

	// パーティクルの描画
	TakeC::TakeCFrameWork::GetParticleManager()->Draw();
}

void GamePlayScene::DrawSprite() {

	// スプライトの描画前処理
	TakeC::SpriteCommon::GetInstance().PreDraw();

	if (sceneStateManager_.GetCurrentStateType() != SceneState::ENEMYDESTROYED) {

		TakeC::TakeCFrameWork::GetSpriteManager()->Draw();
	}
}

//====================================================================
//			影描画処理
//====================================================================
void GamePlayScene::DrawShadow() {

	// ライトカメラのセット
	/*const LightCameraInfo& lightCameraInfo =
		TakeC::TakeCFrameWork::GetLightManager()->GetLightCameraInfo();*/
	// Object3dの影描画前処理
	//Object3dCommon::GetInstance().PreDrawShadowPass();
}

//====================================================================
//			全ての当たり判定のチェック
//====================================================================
void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance().ClearGameCharacter();

	
	CollisionManager::GetInstance().CheckAllCollisionsForGameCharacter();
}