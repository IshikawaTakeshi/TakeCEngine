#include "GameOverScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include <cmath>
#include <algorithm>

void GameOverScene::Initialize() {

	// サウンドデータ
	//gameOverBGM = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/gameOverBGM.wav");
	isSoundPlay = false;
	// GameCamera
	gameOverCamera_ = std::make_unique<Camera>();
	gameOverCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameOverCamera_->SetTranslate({ 0.0f, 20.0f, -60.0f });
	gameOverCamera_->SetRotate({ 0.16f, 0.0f, 0.0f });
	CameraManager::GetInstance()->AddCamera("GameOverCamera", *gameOverCamera_);
	// デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//SkyBox
	skybox_ = std::make_unique<SkyBox>();
	skybox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_pool.obj");
}

void GameOverScene::Finalize() {

	TakeCFrameWork::GetParticleManager()->Finalize();
	
	// サウンドデータの解放
	AudioManager::GetInstance()->SoundUnload(&gameOverBGM);
	// カメラの解放
	CameraManager::GetInstance()->ResetCameras();
}

void GameOverScene::Update() {

	if (!isSoundPlay) {
		//AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), gameOverBGM);
		isSoundPlay = true;
	}
	
	// カメラの更新
	CameraManager::GetInstance()->Update();
	// 天球の更新
	skybox_->Update();

	// パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update();
	
		// シーン遷移
	if (Input::GetInstance()->TriggerButton(0,GamepadButtonType::A)) {

		// シーン切り替え依頼
		AudioManager::GetInstance()->SoundUnload(&gameOverBGM);
		SceneManager::GetInstance()->ChangeScene("TITLE",0.3f);
	}
}

void GameOverScene::UpdateImGui() {
	ImGui::Begin("GameOverScene");
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();	
	TakeCFrameWork::GetParticleManager()->UpdateImGui();
	ImGui::End();
}

void GameOverScene::Draw() {
	skybox_->Draw();

	SpriteCommon::GetInstance()->PreDraw(); // Spriteの描画前処理
	Object3dCommon::GetInstance()->PreDraw(); // Object3dの描画前処理
	
	ParticleCommon::GetInstance()->PreDraw(); // パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); // パーティクルの描画
}
