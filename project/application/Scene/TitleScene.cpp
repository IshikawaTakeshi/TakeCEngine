#include "TitleScene.h"
#include "SceneManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/Math/Quaternion.h"
#include <algorithm>

//====================================================================
//			初期化
//====================================================================
void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"Title_ViewCamera.json");
	camera0_->SetRotationSpeed(1.0f);
	camera0_->SetFollowSpeed(1.0f);
	TakeC::CameraManager::GetInstance().AddCamera("Tcamera0", *camera0_);
	cameraRotateSpeed_ = 0.5f;

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

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(), "skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	playerModel_ = std::make_unique<Object3d>();
	playerModel_->Initialize(&Object3dCommon::GetInstance(), "Player_Model_Ver2.0.gltf");
	playerModel_->SetAnimation(TakeCFrameWork::GetAnimationManager()->FindAnimation("Player_Model_Ver2.0.gltf","Floating"));
	playerModel_->SetRotate( { 0.0f, 180.0f, 0.0f });
	// ShadowMapEffectを無効化
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect", false);
}

//====================================================================
//			終了処理
//====================================================================
void TitleScene::Finalize() {
	titleTextSprite_.reset();
	pushStartUI_.reset();
	skyBox_.reset();
	camera0_.reset();
	camera1_.reset();
	TakeC::CameraManager::GetInstance().ResetCameras();

}

//====================================================================
//			更新処理
//====================================================================
void TitleScene::Update() {

	//カメラの回転(Yaw回転)
	cameraYaw_ += cameraRotateSpeed_ * TakeCFrameWork::GetDeltaTime();
	cameraYaw_ = std::fmod(cameraYaw_, 360.0f); // 360度を超えないようにする
	TakeC::CameraManager::GetInstance().GetActiveCamera()->SetYawRot(cameraYaw_);
	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();

	//SkyBoxの更新
	skyBox_->Update();

	//タイトルテキストの更新
	titleTextSprite_->Update();
	pushStartUI_->Update();

	//プレイヤーモデルの更新
	playerModel_->Update();

	//シーン遷移
	if (TakeC::Input::GetInstance().TriggerButton(0,GamepadButtonType::A)) {
		//シーン切り替え依頼
		//EnemySelectSceneへ
		SceneManager::GetInstance().ChangeScene("GAMEPLAY",1.0f);
	}
}

//====================================================================
//			ImGui更新処理
//====================================================================
void TitleScene::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	//ImGuiの更新
	TakeC::CameraManager::GetInstance().UpdateImGui();
	titleTextSprite_->UpdateImGui("title");
	pushStartUI_->UpdateImGui();
	playerModel_->UpdateImGui("playerModel");

	ImGui::Begin("cameraYaw");
	ImGui::SliderFloat("cameraYaw", &cameraYaw_, 0.0f, 360.0f);
	ImGui::End();
	Object3dCommon::GetInstance().UpdateImGui();

#endif
}

//====================================================================
//			描画処理
//====================================================================
void TitleScene::Draw() {

	//SkyBox描画
	skyBox_->Draw();

	Object3dCommon::GetInstance().Dispatch();
	playerModel_->Dispatch();

	Object3dCommon::GetInstance().PreDraw();
	playerModel_->Draw();

}

void TitleScene::DrawSprite() {
	//タイトルテキスト描画
	SpriteCommon::GetInstance().PreDraw();
	titleTextSprite_->Draw();
	pushStartUI_->Draw();
}

void TitleScene::DrawShadow() {
}
