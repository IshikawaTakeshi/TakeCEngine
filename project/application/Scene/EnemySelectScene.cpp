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
	BGM_ = AudioManager::GetInstance()->LoadSound("SelectSceneBGM.mp3");

	//Camera0
	gameCamera_ = std::make_shared<Camera>();
	gameCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice(),"CameraConfig_SelectScene.json");
	gameCamera_->SetYawRot(2.5f);
	CameraManager::GetInstance()->AddCamera("gameCamera", *gameCamera_);

	//Camera1
	debugCamera_ = std::make_shared<Camera>();
	debugCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice(),"CameraConfig_SelectScene.json");
	debugCamera_->SetIsDebug(true);
	CameraManager::GetInstance()->AddCamera("debugCamera", *debugCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//Animation読み込み
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation", "walk.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation", "Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation", "running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation", "throwAttack.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Models/gltf", "player_singleMesh.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	//キャラクター編集ツール
	characterEditTool_ = std::make_unique<CharacterEditTool>();
	characterEditTool_->Initialize();
}

//====================================================================
//			終了処理
//====================================================================

void EnemySelectScene::Finalize() {
	AudioManager::GetInstance()->SoundUnload(&BGM_); // BGM停止
	CollisionManager::GetInstance()->ClearGameCharacter(); // 当たり判定の解放
	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void EnemySelectScene::Update() {

	//BGM再生
	if (!isSoundPlay_) {
		AudioManager::GetInstance()->SoundPlayWave(BGM_, bgmVolume_,true);
		isSoundPlay_ = true;
	}

	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();
	//キャラクター編集ツールの更新
	characterEditTool_->Update();

	//particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case SceneBehavior::GAMESTART:

			InitializeGameStart();
			break;
		case SceneBehavior::GAMEPLAY:

			InitializeGamePlay();
			break;
		case SceneBehavior::ENEMYDESTROYED:
			InitializeEnemyDestroyed();
			break;
		case SceneBehavior::GAMEOVER:

			InitializeGameOver();
			break;
		case SceneBehavior::GAMECLEAR:

			InitializeGameClear();
			break;
		case SceneBehavior::PAUSE:

			InitializePause();
			break;
		default:
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case SceneBehavior::GAMESTART:

		UpdateGameStart();
		break;
	case SceneBehavior::GAMEPLAY:

		UpdateGamePlay();
		break;
	case SceneBehavior::ENEMYDESTROYED:
		UpdateEnemyDestroyed();
		break;
	case SceneBehavior::GAMEOVER:

		UpdateGameOver();
		break;
	case SceneBehavior::GAMECLEAR:

		UpdateGameClear();
		break;
	case SceneBehavior::PAUSE:

		UpdatePause();
		break;
	default:
		break;
	}

	//メニュー移動リクエスト
	if (characterEditTool_->IsNextMenuRequested() == true) {
		
		//ゲームプレイシーンへ移動
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY", 1.0f);
		//リクエストフラグを下ろす
		characterEditTool_->SetNextMenuRequested(false);
	}
}

void EnemySelectScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();

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
	Object3dCommon::GetInstance()->Dispatch();
	characterEditTool_->DispatchObject();
	
	Object3dCommon::GetInstance()->PreDraw();
	characterEditTool_->DrawObject();
	
	Object3dCommon::GetInstance()->PreDrawAddBlend();
	

#pragma endregion

	TakeCFrameWork::GetWireFrame()->Draw();

	//パーティクルの描画
	TakeCFrameWork::GetParticleManager()->Draw();
}

void EnemySelectScene::DrawSprite() {

	//スプライトの描画前処理
	SpriteCommon::GetInstance()->PreDraw();

	characterEditTool_->DrawUI();
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
	CameraManager::GetInstance()->GetActiveCamera()->SetCameraStateRequest(Camera::GameCameraState::ENEMY_DESTROYED);
	//changeBehaviorTimerを初期化
	changeBehaviorTimer_.Initialize(2.4f, 0.0f);
}

void EnemySelectScene::UpdateEnemyDestroyed() {

	//changeBehaviorTimerの更新
	changeBehaviorTimer_.Update();

	//changeBehaviorTimerが終了したらゲームクリアへ
	if (changeBehaviorTimer_.IsFinished()) {

		//ゲームクリアへ
		behaviorRequest_ = SceneBehavior::GAMECLEAR;

		//ズーム解除
		CameraManager::GetInstance()->GetActiveCamera()->SetCameraStateRequest(Camera::GameCameraState::FOLLOW);
	}
}


//====================================================================
// ゲームオーバー時の処理
//====================================================================
void EnemySelectScene::InitializeGameOver() {

	fadeTimer_ = 2.0f;
	SceneManager::GetInstance()->ChangeScene("GAMEOVER", fadeTimer_);
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
