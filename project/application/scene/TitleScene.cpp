#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include "ImGuiManager.h"

void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera0", *camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera1_->SetTranslate({ -5.0f,0.0f,-10.0f });
	camera1_->SetRotate({ 0.0f,0.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera1", *camera1_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());


	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/images/monsterBall.png");

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "terrain.obj");

	//object
	titleObject = std::make_shared<Object3d>();
	titleObject->Initialize(Object3dCommon::GetInstance(), "terrain.obj");
}

void TitleScene::Finalize() {
	sprite_.reset();
	titleObject.reset();
	camera0_.reset();
	camera1_.reset();
	CameraManager::GetInstance()->ResetCameras();
}

void TitleScene::Update() {
#ifdef _DEBUG
	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	titleObject->UpdateImGui(0);

	ImGui::Begin("TitleScene");
	ImGui::Text("TitleScene");
	ImGui::Text("Press Enter to Start");
	ImGui::End();
	
#endif // DEBUG


	//カメラの更新
	CameraManager::GetInstance()->Update();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();

	Object3dCommon::GetInstance()->PreDrawForObject3d();

}