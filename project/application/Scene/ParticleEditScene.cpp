#include "ParticleEditScene.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"


//=====================================================================
//			初期化
//=====================================================================
void ParticleEditScene::Initialize() {

	// カメラの初期化
	previewCamera_ = std::make_shared<Camera>();
	previewCamera_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"CameraConfig_SelectScene.json");
	previewCamera_->SetIsDebug(true);
	previewCamera_->SetTranslate({ 5.0f,0.0f,-10.0f });
	previewCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	TakeC::CameraManager::GetInstance().AddCamera("previewCamera", *previewCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());
	ParticleCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());

	// ParticleEditorの初期化
	particleEditor_ = std::make_unique<ParticleEditor>();
	particleEditor_->Initialize(&ParticleCommon::GetInstance());

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(), "skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

}

//=====================================================================
//			終了処理
//=====================================================================
void ParticleEditScene::Finalize() {

	// カメラのリセット
	TakeC::CameraManager::GetInstance().ResetCameras();

	// ParticleEditorの解放
	particleEditor_->Finalize();
}

//=====================================================================
//			更新処理
//=====================================================================
void ParticleEditScene::Update() {

	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();
	//SkyBoxの更新
	skyBox_->Update();

	// ParticleEditorの更新
	particleEditor_->Update();
}

//=====================================================================
//			ImGuiの更新
//=====================================================================
void ParticleEditScene::UpdateImGui() {
	// 各種マネージャーのImGui更新
	TakeC::CameraManager::GetInstance().UpdateImGui();
	ParticleCommon::GetInstance().UpdateImGui();
	TakeC::ParticleManager().UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();

	// EditorのImGuiの更新
	particleEditor_->UpdateImGui();
}


//=====================================================================
//			描画処理
//=====================================================================
void ParticleEditScene::Draw() {

	//SkyBox描画
	skyBox_->Draw();

	//グリッド地面の描画
	TakeCFrameWork::GetWireFrame()->DrawGridGround({ 0.0f,0.0f,0.0f }, { 1000.0f, 1000.0f, 1000.0f }, 50);
	TakeCFrameWork::GetWireFrame()->DrawGridBox({
		{-500.0f,-500.0f,-500.0f},{500.0f,500.0f,500.0f } }, 2);
	TakeCFrameWork::GetWireFrame()->Draw();

	// ParticleEditorの描画
	particleEditor_->Draw();
}

void ParticleEditScene::DrawSprite() {

}

void ParticleEditScene::DrawShadow() {
}
