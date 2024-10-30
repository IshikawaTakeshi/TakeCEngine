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
#include "ParticleCommon.h"
#include "TextureManager.h"

Particle3d::~Particle3d() {}

void Particle3d::Initialize(ParticleCommon* particleCommon,const std::string& filePath) {

	particleCommon_ = particleCommon;

	//モデルの読み込み
	SetModel(filePath);
	model_->SetInstanceCount(kNumInstance_);

	//TransformationMatrix用のResource生成
	instancingResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix) * kNumInstance_);

	useSrvIndex_ = model_->GetModelCommon()->GetSrvManager()->Allocate();

	//SRVの生成
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumInstance_,
		sizeof(TransformMatrix),
		instancingResource_.Get(),
		useSrvIndex_
	);

	//TransformationMatrix用
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
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
	//particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	//instancing用のDataを読むためにStructuredBufferのSRVを設定する
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(1, useSrvIndex_);

	model_->DrawForParticle();
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}