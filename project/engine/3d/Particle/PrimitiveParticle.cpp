#include "PrimitiveParticle.h"
#include "DirectXCommon.h"
#include "ModelManager.h"
#include "math/MatrixMath.h"
#include "Vector3Math.h"
#include "camera/CameraManager.h"
#include "base/SrvManager.h"
#include "base/TextureManager.h"
#include "3d/Particle/ParticleCommon.h"
#include "base/TakeCFrameWork.h"
#include <numbers>

PrimitiveParticle::PrimitiveParticle(PrimitiveType type) {
	type_ = type;
}

PrimitiveParticle::~PrimitiveParticle() {}

void PrimitiveParticle::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//ParticleResource生成
	particleResource_ =
		DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);
	particleResource_->SetName(L"PrimitiveParticle::particleResource_");
	//perViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	perViewResource_->SetName(L"PrimitiveParticle::perViewResource_");
	//SRVの生成
	particleSrvIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForGPU),
		particleResource_.Get(),
		particleSrvIndex_
	);
	//perviewInit
	perViewData_->viewProjection = MatrixMath::MakeIdentity4x4();
	perViewData_->billboardMatrix = MatrixMath::MakeIdentity4x4();
	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	TakeCFrameWork::GetPrimitiveDrawer()->CreateVertexData(type_);
	TakeCFrameWork::GetPrimitiveDrawer()->CreateMatrialData(type_, filePath);
}

void PrimitiveParticle::Update() {

	// ランダムエンジンの初期化  
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	numInstance_ = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end(); ) {

		// MEMO: Particleの総出現数だけ繰り返す  
		if (numInstance_ < kNumMaxInstance_) {

			// 寿命が来たら削除  
			if ((*particleIterator).lifeTime_ <= (*particleIterator).currentTime_) {
				particleIterator = particles_.erase(particleIterator);
				continue;
			}

			// particle1つの位置更新  
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
			//経過時間の更新
			(*particleIterator).currentTime_ += kDeltaTime_; 
			//alphaの計算
			float alpha = 1.0f - ((*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

			particleData_[numInstance_].color = {
				(*particleIterator).color_.x,
				(*particleIterator).color_.y,
				(*particleIterator).color_.z,
				alpha
			};
			particleData_[numInstance_].scale = (*particleIterator).transforms_.scale;
			particleData_[numInstance_].rotate = (*particleIterator).transforms_.rotate;
			particleData_[numInstance_].translate = (*particleIterator).transforms_.translate;
			particleData_[numInstance_].velocity = (*particleIterator).velocity_;
			particleData_[numInstance_].lifeTime = (*particleIterator).lifeTime_;
			particleData_[numInstance_].currentTime = (*particleIterator).currentTime_;

			// データをGPUに転送  
			perViewData_->viewProjection = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
			perViewData_->billboardMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetRotationMatrix();

			++numInstance_; // 次のインスタンスに進める  
		}
		++particleIterator; // 次のイテレータに進める  
	}
}

void PrimitiveParticle::UpdateImGui() {}

void PrimitiveParticle::Draw() {


	BaseParticleGroup::Draw();
	//プリミティブの描画
	TakeCFrameWork::GetPrimitiveDrawer()->DrawParticle(particleCommon_->GetGraphicPSO(), numInstance_);
}

Particle PrimitiveParticle::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	
	return BaseParticleGroup::MakeNewParticle(randomEngine, translate);
}

std::list<Particle> PrimitiveParticle::Emit(const Vector3& emitterPos, uint32_t particleCount) {
	
	return BaseParticleGroup::Emit(emitterPos, particleCount);
}

void PrimitiveParticle::SpliceParticles(std::list<Particle> particles) {

	BaseParticleGroup::SpliceParticles(particles);
}
