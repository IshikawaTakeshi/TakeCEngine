#include "Particle3d.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ModelManager.h"
#include "MatrixMath.h"
#include "Model.h"
#include "Camera.h"



Particle3d::~Particle3d() {}

void Particle3d::Initialize(const std::string& filePath) {

	//PSO生成
	pso_ = new PSO();
	pso_->CreatePSOForParticle(dxCommon_->GetDevice(), dxCommon_->GetDXC(), D3D12_CULL_MODE_NONE);
	//ルートシグネチャ取得
	rootSignature_ = pso_->GetRootSignature();

	//モデルの読み込み
	SetModel(filePath);

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix) * kNumInstance_);
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
		Matrix4x4 WorldMatrix = MatrixMath::MakeAffineMatrix(
			transforms_[i].scale,
			transforms_[i].rotate,
			transforms_[i].translate
		);

		Matrix4x4 WVPMatrix = WorldMatrix * camera_->GetViewMatrix() * camera_->GetProjectionMatrix();

		instancingData_[i].WVP = WVPMatrix;
		instancingData_[i].World = WorldMatrix;
	}
}

void Particle3d::Draw() {



	model_->DrawForParticle();
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
