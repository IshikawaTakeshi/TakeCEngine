#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ImGuiManager.h"
#include "CameraManager.h"
#include "Vector3Math.h"

Object3dCommon* Object3dCommon::instance_ = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Object3dCommon();
	}
	return instance_;
	
}

void Object3dCommon::Initialize(DirectXCommon* directXCommon) {

	//DirectXCommon取得
	dxCommon_ = directXCommon;

	//PSO生成
	pso_ = new PSO();
	pso_->CreatePSOForObject3D(dxCommon_->GetDevice(), dxCommon_->GetDXC(), D3D12_CULL_MODE_NONE);

	//ルートシグネチャ取得
	rootSignature_ = pso_->GetRootSignature();

#pragma region "Lighting"
	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLightData));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 0.0f;

	//PointLight用のResourceの作成
	pointLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLightData));
	pointLightData_ = nullptr;
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	pointLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData_->position_ = { 0.0f,2.0f,0.0f };
	pointLightData_->intensity_ = 2.0f;
	pointLightData_->radius_ = 10.0f;
	pointLightData_->decay_ = 1.0f;

#pragma endregion
}

void Object3dCommon::UpdateImGui() {
	ImGui::Text("DirectionalLight");
	ImGui::SliderFloat3("Direction", &directionalLightData_->direction_.x, -1.0f, 1.0f);
	directionalLightData_->direction_ = Vector3Math::Normalize(directionalLightData_->direction_);
	ImGui::DragFloat("dirIntensity", &directionalLightData_->intensity_, 0.01f);
	ImGui::Text("PointLight");
	ImGui::ColorEdit4("Color", &pointLightData_->color_.x);
	ImGui::DragFloat3("Position", &pointLightData_->position_.x, 0.01f);
	ImGui::DragFloat("Intensity", &pointLightData_->intensity_, 0.01f);
	ImGui::SliderFloat("PointLightRadius", &pointLightData_->radius_, 0.0f, 100.0f);
	ImGui::SliderFloat("Decay", &pointLightData_->decay_, 0.0f, 2.0f);
	pso_->UpdateImGui();
}

void Object3dCommon::Finalize() {
	rootSignature_.Reset();
	directionalLightResource_.Reset();
	pointLightResource_.Reset();
	dxCommon_ = nullptr;
	delete pso_;
	pso_ = nullptr;

	delete instance_;
	instance_ = nullptr;
}

void Object3dCommon::PreDraw() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Lighting用のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//カメラ情報のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		4, CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	//pointLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource_->GetGPUVirtualAddress());

}
