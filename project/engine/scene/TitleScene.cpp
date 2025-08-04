#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include <algorithm>

void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.1f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera0", *camera0_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	// Sprite
	titleTextSprite_ = std::make_unique<Sprite>();
	titleTextSprite_->Initialize(SpriteCommon::GetInstance(), "UI/TitleText.png");
	titleTextSprite_->AdjustTextureSize();
	titleTextSprite_->SetPosition({ 200.0f, 256.0f});

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_blueSky.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
}

void TitleScene::Finalize() {
	titleTextSprite_.reset();
	camera0_.reset();
	camera1_.reset();
	CameraManager::GetInstance()->ResetCameras();

}

void TitleScene::Update() {

	//カメラの更新
	CameraManager::GetInstance()->Update();

	//SkyBoxの更新
	skyBox_->Update();

	titleTextSprite_->Update();

	//シーン遷移
	if (Input::GetInstance()->TriggerButton(0,GamepadButtonType::A)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY",1.0f);
	}
}

void TitleScene::UpdateImGui() {
#ifdef _DEBUG
	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	titleTextSprite_->UpdateImGui("title");

#endif
}

void TitleScene::Draw() {

	//SkyBox描画
	skyBox_->Draw();

	SpriteCommon::GetInstance()->PreDraw();
	titleTextSprite_->Draw();
	Object3dCommon::GetInstance()->PreDraw();

}