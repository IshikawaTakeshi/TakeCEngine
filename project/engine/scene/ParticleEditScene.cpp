#include "ParticleEditScene.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"

void ParticleEditScene::Initialize() {

	// カメラの初期化
	previewCamera_ = std::make_shared<Camera>();
	previewCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	previewCamera_->SetIsDebug(true);
	previewCamera_->SetTranslate({ 5.0f,0.0f,-10.0f });
	previewCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("previewCamera", *previewCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	particleEditor_ = std::make_unique<ParticleEditor>();
	particleEditor_->Initialize(TakeCFrameWork::GetParticleManager(), ParticleCommon::GetInstance());

}

void ParticleEditScene::Finalize() {

	// カメラのリセット
	CameraManager::GetInstance()->ResetCameras();
	// ParticleManagerのリセット
	//TakeCFrameWork::GetParticleManager()
	// ParticleEditorの解放
	particleEditor_->Finalize();
}

void ParticleEditScene::Update() {

	// カメラの更新
	previewCamera_->Update();
	// ParticleEditorの更新
	particleEditor_->Update();
}

void ParticleEditScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	ParticleCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

	// ImGuiの更新
	particleEditor_->UpdateImGui();
}

void ParticleEditScene::Draw() {
	// ParticleEditorの描画
	particleEditor_->Draw();
}
