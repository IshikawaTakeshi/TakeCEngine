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
	gameOverCamera_->SetRotate({ 0.9f, -0.01f, 0.03f,0.36f });
	CameraManager::GetInstance()->AddCamera("GameOverCamera", *gameOverCamera_);
	// デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//SkyBox
	skybox_ = std::make_unique<SkyBox>();
	skybox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_blueSky.obj");

	//Spriteの初期化
	gameOverTextSprite_ = std::make_unique<Sprite>();
	gameOverTextSprite_->Initialize(SpriteCommon::GetInstance(), "UI/GameOverText.png");
	gameOverTextSprite_->SetPosition({ 50.0f, 50.0f });
	gameOverTextSprite_->AdjustTextureSize();

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

	// スプライトの更新
	gameOverTextSprite_->Update();

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
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();	
	TakeCFrameWork::GetParticleManager()->UpdateImGui();
	gameOverTextSprite_->UpdateImGui("GameOverTextSprite");

}

void GameOverScene::Draw() {
	skybox_->Draw();

	SpriteCommon::GetInstance()->PreDraw(); // Spriteの描画前処理
	gameOverTextSprite_->Draw(); // スプライトの描画

	Object3dCommon::GetInstance()->PreDraw(); // Object3dの描画前処理
	
	ParticleCommon::GetInstance()->PreDraw(); // パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); // パーティクルの描画
}
