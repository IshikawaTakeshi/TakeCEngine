#include "GameClearScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include <cmath>
#include <algorithm>

void GameClearScene::Initialize() {

	// サウンドデータ
	//gameClearBGM = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/gameClearBGM.wav");
	isSoundPlay = false;
	// GameCamera
	gameClearCamera_ = std::make_unique<Camera>();
	gameClearCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameClearCamera_->SetTranslate({ 43.0f, 1.5f, -20.0f });
	gameClearCamera_->SetRotate({ -0.03f, -0.5f, -0.02f,0.85f });
	CameraManager::GetInstance()->AddCamera("GameOverCamera", *gameClearCamera_);
	// デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//SkyBox
	skybox_ = std::make_unique<SkyBox>();
	skybox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_blueSky.obj");
	skybox_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });


	//whiteOutSprite
	clearTextSprite_ = std::make_unique<Sprite>();
	clearTextSprite_->Initialize(SpriteCommon::GetInstance(), "UI/GameClearText.png");
	clearTextSprite_->SetPosition({ 640.0f, 360.0f });
	clearTextSprite_->AdjustTextureSize();

}

void GameClearScene::Finalize() {
	// サウンドデータの解放
	AudioManager::GetInstance()->SoundUnload(&gameClearBGM);
	// カメラの解放
	CameraManager::GetInstance()->ResetCameras();
}

void GameClearScene::Update() {

	// オーディオ再生
	if (!isSoundPlay) {
		//AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), gameClearBGM);
		isSoundPlay = true;
	}
	// カメラの更新
	CameraManager::GetInstance()->Update();
	// 天球の更新
	skybox_->Update();

	clearTextSprite_->Update();
	// パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update();

		// シーン遷移
		if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::A)) {

			SceneManager::GetInstance()->ChangeScene("TITLE", 1.0f);
		}
}

void GameClearScene::UpdateImGui() {
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();
	clearTextSprite_->UpdateImGui("ClearText");
}

void GameClearScene::Draw() {

	skybox_->Draw(); // 天球の描画

	SpriteCommon::GetInstance()->PreDraw(); // Spriteの描画前処理
	clearTextSprite_->Draw();
	Object3dCommon::GetInstance()->PreDraw();   //Object3dの描画前処理

	TakeCFrameWork::GetWireFrame()->DrawGridBox({
		{-500.0f,-500.0f,-500.0f},{500.0f,500.0f,500.0f } }, 2);
	TakeCFrameWork::GetWireFrame()->Draw();

	//ParticleCommon::GetInstance()->PreDraw(); // パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); // パーティクルの描画
}
