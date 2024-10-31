#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"

void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera0", *camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->SetTranslate({ -5.0f,0.0f,-10.0f });
	camera1_->SetRotate({ 0.0f,0.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera1", *camera1_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());


	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/monsterBall.png");

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("Title.obj");

	//object
	titleObject = std::make_shared<Object3d>();
	titleObject->Initialize(Object3dCommon::GetInstance(), "Title.obj");
	titleObject->SetScale({ 0.1f,0.1f,0.1f});
	titleObject->SetRotation({ -1.5f,0.0f,0.0f });
}

void TitleScene::Finalize() {
	sprite_.reset();
	titleObject.reset();
	CameraManager::GetInstance()->ResetCameras();
}

void TitleScene::Update() {
#ifdef _DEBUG
	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite_->UpdateImGui(0);
	titleObject->UpdateImGui(0);
#endif // DEBUG


	//カメラの更新
	CameraManager::GetInstance()->Update();

	sprite_->Update();
	titleObject->Update();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();
	Object3dCommon::GetInstance()->PreDraw();
	sprite_->Draw();
	titleObject->Draw();
}