#include "Object3dCommon.h"
#include "DirectXCommon.h"

#include "ImGuiManager.h"
#include "CameraManager.h"
#include "Vector3Math.h"
#include <numbers>
#include <algorithm>

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
	psoForObject3d_ = std::make_unique<PSO>();
	psoForSkinningObject3d_ = std::make_unique<PSO>();

	psoForObject3d_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.VS.hlsl");
	psoForObject3d_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.PS.hlsl");

	psoForSkinningObject3d_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.VS.hlsl");
	psoForSkinningObject3d_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.PS.hlsl");
	psoForSkinningObject3d_->CompileComputeShader(dxCommon_->GetDXC(), L"Resources/shaders/Skinning.CS.hlsl");

	//ルートシグネチャ取得
	rootSignatureForObject3d_ = psoForObject3d_->GetGraphicRootSignature();
	rootSignatureForSkinningObject3d_[graphic] = psoForSkinningObject3d_->GetGraphicRootSignature();
	rootSignatureForSkinningObject3d_[compute] = psoForSkinningObject3d_->GetComputeRootSignature();

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
	directionalLightData_->intensity_ = 1.0f;

	//PointLight用のResourceの作成
	pointLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLightData));
	pointLightData_ = nullptr;
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	pointLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData_->position_ = { 0.0f,2.0f,0.0f };
	pointLightData_->intensity_ = 0.0f;
	pointLightData_->radius_ = 10.0f;
	pointLightData_->decay_ = 1.0f;

	//SpotLight用のResourceの作成
	spotLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLightData));
	spotLightData_ = nullptr;
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	spotLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData_->position_ = { 2.0f,2.0f,0.0f };
	spotLightData_->direction_ = { -1.0f,-1.0f,0.0f };
	spotLightData_->distance_ = 7.0f;
	spotLightData_->intensity_ = 4.0f;
	spotLightData_->decay_ = 2.0f;
	spotLightData_->cosAngle_ = std::cosf(std::numbers::pi_v<float> / 3.0f);
	spotLightData_->penumbraAngle_ = std::numbers::pi_v<float> / 6.0f;

#pragma endregion
}

void Object3dCommon::UpdateImGui() {
	ImGui::Begin("Lighting");
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
	ImGui::Text("SpotLight");
	ImGui::ColorEdit4("SpotColor", &spotLightData_->color_.x);
	ImGui::DragFloat3("SpotPosition", &spotLightData_->position_.x, 0.01f);
	ImGui::DragFloat3("SpotDirection", &spotLightData_->direction_.x, 0.01f);
	spotLightData_->direction_ = Vector3Math::Normalize(spotLightData_->direction_);
	ImGui::DragFloat("SpotDistance", &spotLightData_->distance_, 0.01f);
	ImGui::DragFloat("SpotIntensity", &spotLightData_->intensity_, 0.01f);
	ImGui::SliderFloat("SpotDecay", &spotLightData_->decay_, 0.0f, 2.0f);
	ImGui::SliderAngle("SpotCosAngle", &spotLightData_->cosAngle_);
	ImGui::SliderAngle("SpotPenumbraAngle", &spotLightData_->penumbraAngle_);
	ImGui::End();
}

void Object3dCommon::Finalize() {
	directionalLightResource_.Reset();
	pointLightResource_.Reset();
	dxCommon_ = nullptr;

	delete instance_;
	instance_ = nullptr;
}

void Object3dCommon::PreDrawForObject3d() {

	
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignatureForObject3d_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(psoForObject3d_->GetGraphicPipelineState());

	//DirectionalLight
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//カメラ情報のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		5, CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	//pointLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(7, pointLightResource_->GetGPUVirtualAddress());
	//spotLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, spotLightResource_->GetGPUVirtualAddress());

}

void Object3dCommon::PreDrawForSkinningObject3d() {

	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(psoForSkinningObject3d_->GetGraphicPipelineState());
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignatureForSkinningObject3d_[graphic].Get());

	//DirectionalLight
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//カメラ情報のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		5, CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	//pointLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(7, pointLightResource_->GetGPUVirtualAddress());
	//spotLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, spotLightResource_->GetGPUVirtualAddress());

}

void Object3dCommon::DisPatch() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(psoForSkinningObject3d_->GetComputePipelineState());
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureForSkinningObject3d_[compute].Get());
}
