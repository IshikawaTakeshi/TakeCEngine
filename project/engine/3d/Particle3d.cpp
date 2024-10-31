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
#include <numbers>


Particle3d::~Particle3d() {}

void Particle3d::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//モデルの読み込み
	SetModel(filePath);
	
	//TransformationMatrix用のResource生成
	instancingResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);

	useSrvIndex_ = model_->GetModelCommon()->GetSrvManager()->Allocate();

	//SRVの生成
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForGPU),
		instancingResource_.Get(),
		useSrvIndex_
	);

	//TransformationMatrix用
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	//単位行列を書き込んでおく
	for (uint32_t i = 0; i < kNumMaxInstance_; i++) {
		instancingData_[i].WVP = MatrixMath::MakeIdentity4x4();
		instancingData_[i].World = MatrixMath::MakeIdentity4x4();
		instancingData_[i].Color = { 1.0f,1.0f,1.0f,1.0f };
	}

	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	for (uint32_t i = 0; i < kNumMaxInstance_; i++) {
		particles_[i] = MakeNewParticle(randomEngine);
	}
}

void Particle3d::Update() {

	numInstance_ = 0;
	for (uint32_t index = 0; index < kNumMaxInstance_; index++) {
		if (particles_[index].lifeTime_ <= particles_[index].currentTime_) {
			continue;
		}
		//新しいパーティクルの生成
		if (isSpawn_) {
			std::random_device seedGenerator;
			std::mt19937 randomEngine(seedGenerator());
			particles_[index] = MakeNewParticle(randomEngine);
		}

		//位置の更新
		particles_[index].transforms_.translate += particles_[index].velocity_ * kDeltaTime_;
		particles_[index].currentTime_ += kDeltaTime_; //経過時間の更新
		float alpha = 1.0f - (particles_[index].currentTime_ / particles_[index].lifeTime_);

		//アフィン行列の更新
		
		worldMatrix_ = MatrixMath::MakeAffineMatrix(
			particles_[index].transforms_.scale,
			particles_[index].transforms_.rotate,
			particles_[index].transforms_.translate
		);

		if (isBillboard_) {
			worldMatrix_ = MatrixMath::MakeAffineMatrix(
				particles_[index].transforms_.scale,
				CameraManager::GetInstance()->GetActiveCamera()->GetRotate(),
				particles_[index].transforms_.translate
			);
		}

		//wvpの更新
		if (camera_) {
			
			const Matrix4x4& viewProjectionMatrix = MatrixMath::MakeRotateYMatrix(0) *
				CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
			WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
			
		} else {
			WVPMatrix_ = worldMatrix_;
		}

		instancingData_[numInstance_].WVP = WVPMatrix_;
		instancingData_[numInstance_].World = worldMatrix_;
		instancingData_[numInstance_].Color = particles_[index].color_;
		instancingData_[numInstance_].Color.w = alpha;
		++numInstance_;
	}
}

void Particle3d::UpdateImGui() {
	ImGui::Begin("Particle3d");
	ImGui::DragFloat3("Scale", &particles_[0].transforms_.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Rotate", &particles_[0].transforms_.rotate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Translate", &particles_[0].transforms_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::Checkbox("Billboard", &isBillboard_);
	if (ImGui::Button("Spawn")) {
		isSpawn_ = true;
	}
	particleCommon_->GetPSO()->UpdateImGui();
	ImGui::End();
}

void Particle3d::Draw() {

	//instancing用のDataを読むためにStructuredBufferのSRVを設定する
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(1, useSrvIndex_);

	model_->DrawForParticle(numInstance_);
}

Particle3d::Particle Particle3d::MakeNewParticle(std::mt19937& randomEngine) {
	
	std::uniform_real_distribution<float> distribution(-0.1f, 1.0f); //位置と速度を[-1,1]でランダムに設定
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);    //色を[0,1]でランダムに設定
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);    //寿命を[1,3]でランダムに設定

	Particle particle;
	particle.transforms_.scale = { 1.0f,1.0f,1.0f };
	particle.transforms_.rotate = { 0.0f,0.0f,0.0f };
	particle.transforms_.translate = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.velocity_ = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color_ = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifeTime_ = distTime(randomEngine);
	particle.currentTime_ = 0.0f;
	return particle;
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}