#include "EnemySelectScene.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "Vector3Math.h"
#include "ImGuiManager.h"
#include "Collision/CollisionManager.h"
#include "MyGame.h"
#include <format>
#include <numbers>
//====================================================================
//			初期化
//====================================================================

void EnemySelectScene::Initialize() {

	//BGM読み込み
	BGM_ = AudioManager::GetInstance().LoadSound("SelectSceneBGM.mp3");

	//Camera0
	gameCamera_ = std::make_unique<Camera>();
	gameCamera_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"CameraConfig_SelectScene.json");
	gameCamera_->SetYawRot(2.5f);
	TakeC::CameraManager::GetInstance().AddCamera("gameCamera", gameCamera_.get());

	//Camera1
	debugCamera_ = std::make_unique<Camera>();
	debugCamera_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"CameraConfig_SelectScene.json");
	debugCamera_->SetIsDebug(true);
	TakeC::CameraManager::GetInstance().AddCamera("debugCamera", debugCamera_.get());

	//デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());
	ParticleCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(), "skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	//キャラクター編集ツール
	characterEditTool_ = std::make_unique<CharacterEditTool>();
	characterEditTool_->Initialize();
}

//====================================================================
//			終了処理
//====================================================================

void EnemySelectScene::Finalize() {
	AudioManager::GetInstance().SoundUnload(&BGM_); // BGM停止
	CollisionManager::GetInstance().ClearGameCharacter(); // 当たり判定の解放
	TakeC::CameraManager::GetInstance().ResetCameras(); //カメラのリセット
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void EnemySelectScene::Update() {

	//BGM再生
	if (!isSoundPlay_) {
		AudioManager::GetInstance().SoundPlayWave(BGM_, bgmVolume_,true);
		isSoundPlay_ = true;
	}

	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();
	//SkyBoxの更新
	skyBox_->Update();
	//キャラクター編集ツールの更新
	characterEditTool_->Update();

	//particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case SceneState::GAMESTART:

			InitializeGameStart();
			break;
		case SceneState::GAMEPLAY:

			InitializeGamePlay();
			break;
		case SceneState::ENEMYDESTROYED:
			InitializeEnemyDestroyed();
			break;
		case SceneState::GAMEOVER:

			InitializeGameOver();
			break;
		case SceneState::GAMECLEAR:

			InitializeGameClear();
			break;
		case SceneState::PAUSE:

			InitializePause();
			break;
		default:
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case SceneState::GAMESTART:

		UpdateGameStart();
		break;
	case SceneState::GAMEPLAY:

		UpdateGamePlay();
		break;
	case SceneState::ENEMYDESTROYED:
		UpdateEnemyDestroyed();
		break;
	case SceneState::GAMEOVER:

		UpdateGameOver();
		break;
	case SceneState::GAMECLEAR:

		UpdateGameClear();
		break;
	case SceneState::PAUSE:

		UpdatePause();
		break;
	default:
		break;
	}

	//メニュー移動リクエスト
	if (characterEditTool_->IsNextMenuRequested() == true) {
		
		//ゲームプレイシーンへ移動
		SceneManager::GetInstance().ChangeScene("GAMEPLAY", 1.0f);
		//リクエストフラグを下ろす
		characterEditTool_->SetNextMenuRequested(false);
	}
}

void EnemySelectScene::UpdateImGui() {

	TakeC::CameraManager::GetInstance().UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();
	TakeCFrameWork::GetSpriteManager()->UpdateImGui();
	ImGui::Begin("Level Objects");
	characterEditTool_->UpdateImGui();
	ImGui::End();

	//particleEmitter_->UpdateImGui();
}

//====================================================================
//			描画処理
//====================================================================

void EnemySelectScene::Draw() {

	skyBox_->Draw();    //天球の描画

#pragma region Object3d描画

	//Object3dの描画前処理
	Object3dCommon::GetInstance().Dispatch();
	characterEditTool_->DispatchObject();
	
	Object3dCommon::GetInstance().PreDraw();
	characterEditTool_->DrawObject();
	
	Object3dCommon::GetInstance().PreDrawAddBlend();
	

#pragma endregion

	//ワイヤーフレームの描画
	TakeCFrameWork::GetWireFrame()->Draw();

	//パーティクルの描画
	TakeCFrameWork::GetParticleManager()->Draw();
}

void EnemySelectScene::DrawSprite() {

	//スプライトの描画前処理
	SpriteCommon::GetInstance().PreDraw();

	characterEditTool_->DrawUI();
}

void EnemySelectScene::DrawShadow() {
}

//====================================================================
// ゲームスタート時の処理
//====================================================================

void EnemySelectScene::InitializeGameStart() {}

void EnemySelectScene::UpdateGameStart() {}

//====================================================================
// ゲームプレイ時の処理
//====================================================================
void EnemySelectScene::InitializeGamePlay() {}

void EnemySelectScene::UpdateGamePlay() {

}

//====================================================================
// 敵撃破時の処理
//====================================================================

void EnemySelectScene::InitializeEnemyDestroyed() {

	//スローモーションにする
	MyGame::RequestTimeScale(-1.0f, 1.0f, 1.0f);
	//カメラをズームする
	TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(Camera::GameCameraState::ENEMY_DESTROYED);
	//changeBehaviorTimerを初期化
	changeBehaviorTimer_.Initialize(2.4f, 0.0f);
}

void EnemySelectScene::UpdateEnemyDestroyed() {

	//changeBehaviorTimerの更新
	changeBehaviorTimer_.Update();

	//changeBehaviorTimerが終了したらゲームクリアへ
	if (changeBehaviorTimer_.IsFinished()) {

		//ゲームクリアへ
		behaviorRequest_ = SceneState::GAMECLEAR;

		//ズーム解除
		TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(Camera::GameCameraState::FOLLOW);
	}
}


//====================================================================
// ゲームオーバー時の処理
//====================================================================
void EnemySelectScene::InitializeGameOver() {

	fadeTimer_ = 2.0f;
	SceneManager::GetInstance().ChangeScene("GAMEOVER", fadeTimer_);
}

void EnemySelectScene::UpdateGameOver() {}
//====================================================================
// ゲームクリア時の処理
//====================================================================
void EnemySelectScene::InitializeGameClear() {

	//スローモーション解除
	MyGame::RequestTimeScale(1.0f, 0.6f, 0.0f);

	//フェード処理の開始
	fadeTimer_ = 4.0f;
	//SceneManager::GetInstance()->ChangeScene("GAMECLEAR", fadeTimer_);
}

void EnemySelectScene::UpdateGameClear() {

}

//====================================================================
// ポーズ時の処理
//====================================================================
void EnemySelectScene::InitializePause() {}

void EnemySelectScene::UpdatePause() {}
