#include "Particle3d.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ModelManager.h"
#include "MatrixMath.h"
#include "Model.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "CameraManager.h"
#include "SrvManager.h"
#include "ModelCommon.h"

Particle3d::~Particle3d() {}

void Particle3d::Initialize(const std::string& filePath) {

	//モデルの読み込み
	SetModel(filePath);

	//PSO生成
	pso_ = new PSO();
	pso_->CreatePSOForParticle(
		model_->GetModelCommon()->GetDirectXCommon()->GetDevice(),
		model_->GetModelCommon()->GetDirectXCommon()->GetDXC(),
		D3D12_CULL_MODE_NONE);
	//ルートシグネチャ取得
	rootSignature_ = pso_->GetRootSignature();

	//SRVの生成
	model_->GetModelCommon()->GetSrvManager()->CreateSRVforStructuredBuffer(
		10,
		sizeof(TransformMatrix),
		instancingResource_.Get(),
		model_->GetModelCommon()->GetSrvManager()->Allocate() + 1
	);



	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(model_->GetModelCommon()->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix) * kNumInstance_);
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	//単位行列を書き込んでおく
	for (uint32_t i = 0; i < kNumInstance_; i++) {
		instancingData_[i].WVP = MatrixMath::MakeIdentity4x4();
		instancingData_[i].World = MatrixMath::MakeIdentity4x4();
	}

	for (uint32_t i = 0; i < kNumInstance_; i++) {
		transforms_[i].scale = { 1.0f,1.0f,1.0f };
		transforms_[i].rotate = { 0.0f,0.0f,0.0f };
		transforms_[i].translate = { i * 0.1f,i * 0.1f,i * 0.1f };
	}
}

void Particle3d::Update() {

	for (uint32_t i = 0; i < kNumInstance_; i++) {
		//アフィン行列の更新
		worldMatrix_ = MatrixMath::MakeAffineMatrix(
			transforms_[i].scale,
			transforms_[i].rotate,
			transforms_[i].translate
		);

		//wvpの更新
		if (camera_) {
			const Matrix4x4& viewProjectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
			WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
		} else {
			WVPMatrix_ = worldMatrix_;
		}

		
		instancingData_[i].WVP = WVPMatrix_;
		instancingData_[i].World = worldMatrix_;
	}
}

void Particle3d::UpdateImGui() {
	ImGui::Begin("Particle3d");
	ImGui::DragFloat3("Scale", &transforms_[0].scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Rotate", &transforms_[0].rotate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Translate", &transforms_[0].translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::End();
}

void Particle3d::Draw() {

	//wvp用のCBufferの場所を指定
	model_->GetModelCommon()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定
	model_->GetModelCommon()->GetSrvManager()->SetGraphicsRootDescriptorTable(2, 2);

	model_->DrawForParticle();
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
