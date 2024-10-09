#include "TitleScene.h"

void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera(*camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->SetTranslate({ 5.0f,0.0f,-10.0f });
	camera1_->SetRotate({ 0.0f,-0.4f,0.0f });
	CameraManager::GetInstance()->AddCamera(*camera1_);

	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/monsterBall.png");
}

void TitleScene::Finalize() {
	sprite_.reset();
}

void TitleScene::Update() {
	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite_->UpdateImGui(0);

	//入力更新
	Input::GetInstance()->Update();

	//カメラの更新
	CameraManager::GetInstance()->Update();

	//カメラの切り替え
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		CameraManager::GetInstance()->SetActiveCamera(0);
	} else if (Input::GetInstance()->TriggerKey(DIK_2)) {
		CameraManager::GetInstance()->SetActiveCamera(1);
	}

	sprite_->Update();
}

void TitleScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();
	sprite_->Draw();
}