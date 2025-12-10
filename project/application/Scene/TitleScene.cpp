#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include "engine/base/TakeCFrameWork.h"
#include <algorithm>

//====================================================================
//			初期化
//====================================================================
void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"CameraConfig_TitleScene.json");
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.1f,0.0f,0.0f });
	TakeC::CameraManager::GetInstance().AddCamera("Tcamera0", *camera0_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());

	// Sprite
	titleTextSprite_ = std::make_unique<Sprite>();
	titleTextSprite_->Initialize(&SpriteCommon::GetInstance(), "UI/TitleText.png");
	titleTextSprite_->AdjustTextureSize();
	titleTextSprite_->SetTranslate({ 200.0f, 256.0f});

	// 「PRESS START」スプライトUI
	pushStartUI_ = std::make_unique<PushStartUI>();
	pushStartUI_->Initialize();

	TakeCFrameWork::GetAnimationManager()->LoadAnimation("Models/gltf", "Deer.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(), "skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
}

//====================================================================
//			終了処理
//====================================================================
void TitleScene::Finalize() {
	titleTextSprite_.reset();
	pushStartUI_.reset();
	camera0_.reset();
	camera1_.reset();
	TakeC::CameraManager::GetInstance().ResetCameras();

}

//====================================================================
//			更新処理
//====================================================================
void TitleScene::Update() {

	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();

	//SkyBoxの更新
	skyBox_->Update();

	//タイトルテキストの更新
	titleTextSprite_->Update();
	pushStartUI_->Update();

	//シーン遷移
	if (TakeC::Input::GetInstance().TriggerButton(0,GamepadButtonType::A)) {
		//シーン切り替え依頼
		//EnemySelectSceneへ
		SceneManager::GetInstance().ChangeScene("ENEMYSELECT",1.0f);
	}
}

//====================================================================
//			ImGui更新処理
//====================================================================
void TitleScene::UpdateImGui() {
#ifdef _DEBUG
	//ImGuiの更新
	TakeC::CameraManager::GetInstance().UpdateImGui();
	titleTextSprite_->UpdateImGui("title");
	pushStartUI_->UpdateImGui();
	

#endif
}

//====================================================================
//			描画処理
//====================================================================
void TitleScene::Draw() {

	//SkyBox描画
	skyBox_->Draw();

	
	//phaseMessageUI_->DrawObject();
	Object3dCommon::GetInstance().Dispatch();

	Object3dCommon::GetInstance().PreDraw();

}

void TitleScene::DrawSprite() {
	//タイトルテキスト描画
	SpriteCommon::GetInstance().PreDraw();
	titleTextSprite_->Draw();
	pushStartUI_->Draw();
}
