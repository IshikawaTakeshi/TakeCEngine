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

	//Emitter初期化
	emitter_.transforms_.translate = { 0.0f,0.0f,0.0f };
	emitter_.transforms_.rotate = { 0.0f,0.0f,0.0f };
	emitter_.transforms_.scale = { 1.0f,1.0f,1.0f };
	emitter_.particleCount_ = 3;
	emitter_.frequency_ = 2.0f;
	emitter_.frequencyTime_ = 0.0f;

	//Acceleration初期化
	accelerationField_.acceleration_ = { 0.0f,-9.8f,0.0f };
	accelerationField_.aabb_.SetMin( { -1.0f,-1.0f,-1.0f });
	accelerationField_.aabb_.SetMax( { 1.0f, 1.0f, 1.0f });

	//TransformationMatrix用
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	//単位行列を書き込んでおく
	for (uint32_t i = 0; i < kNumMaxInstance_; i++) {
		instancingData_[i].WVP = MatrixMath::MakeIdentity4x4();
		instancingData_[i].World = MatrixMath::MakeIdentity4x4();
		instancingData_[i].Color = { 1.0f,1.0f,1.0f,1.0f };
	}

	//ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
}

void Particle3d::Update() {

	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	numInstance_ = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end(); ) {
		if ((*particleIterator).lifeTime_ <= (*particleIterator).currentTime_) {
			particleIterator = particles_.erase(particleIterator); //寿命が来たら削除
			continue;
		}

		if (numInstance_ < kNumMaxInstance_) {

			//エミッターの更新
			emitter_.frequencyTime_ += kDeltaTime_;
			if(emitter_.frequency_ <= emitter_.frequencyTime_) {
				particles_.splice(particles_.end(), Emit(emitter_, randomEngine));
				emitter_.frequencyTime_ -= emitter_.frequency_; //余計に過ぎた時間も加味して頻度計算する
			}

			//位置の更新
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
			(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新
			float alpha = 1.0f - ((*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

			//アフィン行列の更新

			worldMatrix_ = MatrixMath::MakeAffineMatrix(
				(*particleIterator).transforms_.scale,
				(*particleIterator).transforms_.rotate,
				(*particleIterator).transforms_.translate
			);

			if (isBillboard_) {
				worldMatrix_ = MatrixMath::MakeAffineMatrix(
					(*particleIterator).transforms_.scale,
					CameraManager::GetInstance()->GetActiveCamera()->GetRotate(),
					(*particleIterator).transforms_.translate
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
			instancingData_[numInstance_].Color = (*particleIterator).color_;
			instancingData_[numInstance_].Color.w = alpha;
			++numInstance_;
		}
		++particleIterator; //次のイテレータに進める
	}
}

void Particle3d::UpdateImGui() {
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	ImGui::Begin("Particle3d");
	ImGui::Checkbox("Billboard", &isBillboard_);
	if (ImGui::Button("SpawnParticle")) {
		particles_.splice(particles_.end(), Emit(emitter_, randomEngine));
	}
	ImGui::Text("ParticleCount:%d", particles_.size());
	ImGui::Text("Emitter");
	ImGui::SliderFloat3("Translate", &emitter_.transforms_.translate.x, -10.0f, 10.0f);
	particleCommon_->GetPSO()->UpdateImGui();
	ImGui::End();
}

void Particle3d::Draw() {

	//instancing用のDataを読むためにStructuredBufferのSRVを設定する
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(1, useSrvIndex_);

	model_->DrawForParticle(numInstance_);
}

Particle Particle3d::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	
	std::uniform_real_distribution<float> distribution(-0.1f, 1.0f); //位置と速度を[-1,1]でランダムに設定
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);    //色を[0,1]でランダムに設定
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);    //寿命を[1,3]でランダムに設定

	Particle particle;
	particle.transforms_.scale = { 1.0f,1.0f,1.0f };
	particle.transforms_.rotate = { 0.0f,0.0f,0.0f };
	Vector3 randomTranslate = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.transforms_.translate = translate + randomTranslate;
	particle.velocity_ = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color_ = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifeTime_ = distTime(randomEngine);
	particle.currentTime_ = 0.0f;
	return particle;
}

std::list<Particle> Particle3d::Emit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particle> particles;
	for (uint32_t count = 0; count < emitter.particleCount_; ++count) {
		particles.push_back(MakeNewParticle(randomEngine,emitter.transforms_.translate));
	}
	return particles;
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}