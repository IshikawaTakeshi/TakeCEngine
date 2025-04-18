#include "Particle3d.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ModelManager.h"
#include "MatrixMath.h"
#include "Vector3Math.h"
#include "Model.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "CameraManager.h"
#include "SrvManager.h"
#include "ModelCommon.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "Input.h"
#include <numbers>


Particle3d::~Particle3d() {
	particleResource_.Reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Particle3d::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//モデルの読み込み
	SetModel(filePath);

	//instancing用のResource生成
	particleResource_ =
		DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);

	//perViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));

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

	//Acceleration初期化
	accelerationField_.acceleration_ = { 0.0f,-9.8f,0.0f };
	accelerationField_.aabb_.min = { -3.0f,-3.0f,-3.0f };
	accelerationField_.aabb_.max = { 3.0f, 3.0f, 3.0f };

	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	//カメラのセット
	camera_ = particleCommon_->GetDefaultCamera();

	//属性初期化
	particleAttributes_.scale = { 1.0f,1.0f,1.0f };
	particleAttributes_.positionRange = { 2.0f,2.0f };
	particleAttributes_.scaleRange = { 1.0f,1.0f };
	particleAttributes_.rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	particleAttributes_.velocityRange = { 0.0f,1.0f };
	particleAttributes_.colorRange = { 0.0f,1.0f };
	particleAttributes_.lifetimeRange = { 5.0f,5.0f };
	particleAttributes_.editColor = true;
	particleAttributes_.color = { 0.9f,0.3f,0.9f };
}

//=============================================================================
// 更新処理
//=============================================================================

void Particle3d::Update() {
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

			// 加速フィールドの位置更新  
			//if (IsCollision(accelerationField_.aabb_, (*particleIterator).transforms_.translate)) {
			//	(*particleIterator).velocity_ += accelerationField_.acceleration_ * kDeltaTime_;
			//}

			// particle1つの位置更新  
			EmitMove(particleIterator);
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
			perViewData_->viewProjection = camera_->GetViewProjectionMatrix();
			perViewData_->billboardMatrix = camera_->GetRotationMatrix();

			

			++numInstance_; // 次のインスタンスに進める  
		}
		++particleIterator; // 次のイテレータに進める  
	}
}

//=============================================================================
// ImGuiの更新
//=============================================================================

void Particle3d::UpdateImGui() {
#ifdef _DEBUG
	ImGui::Text("Particle3d");
	ImGui::Text("ParticleCount:%d", numInstance_);
	particleCommon_->GetGraphicPSO()->UpdateImGui();
	ImGui::Separator();
	ImGui::DragFloat3("Scale", &particleAttributes_.scale.x, 0.01f);
	ImGui::DragFloat2("PositionRange", &particleAttributes_.positionRange.min, 0.01f);
	ImGui::DragFloat2("VelocityRange", &particleAttributes_.velocityRange.min, 0.01f);
	ImGui::DragFloat2("ColorRange", &particleAttributes_.colorRange.min, 0.01f);
	ImGui::DragFloat2("LifetimeRange", &particleAttributes_.lifetimeRange.min, 0.01f);
	ImGui::Checkbox("EditColor", &particleAttributes_.editColor);
	if (particleAttributes_.editColor) {
		ImGui::ColorEdit3("Color", &particleAttributes_.color.x);
	}
	ImGui::Separator();
	ImGui::Text("AccelerationField");
	ImGui::DragFloat3("Acceleration", &accelerationField_.acceleration_.x, 0.01f);
	ImGui::DragFloat3("Position", &accelerationField_.position_.x, 0.01f);
	ImGui::DragFloat3("AABBMin", &accelerationField_.aabb_.min.x, 0.01f);
	ImGui::DragFloat3("AABBMax", &accelerationField_.aabb_.max.x, 0.01f);
	ImGui::Separator();

#endif // _DEBUG
}

//=============================================================================
// 描画処理
//=============================================================================

void Particle3d::Draw() {

	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gPerView"), perViewResource_->GetGPUVirtualAddress());
	
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gParticle"), particleSrvIndex_);

	model_->DrawForParticle(numInstance_);
}

void Particle3d::DrawPrimitive() {
	// perViewResource
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gPerView"), perViewResource_->GetGPUVirtualAddress());
	// particleResource
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gParticle"), particleSrvIndex_);

	//プリミティブの描画
	TakeCFrameWork::GetPrimitiveDrawer()->DrawParticle(particleCommon_->GetGraphicPSO(), numInstance_);
}

//=============================================================================
// パーティクルの生成
//=============================================================================

Particle Particle3d::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {

	//スケールをランダムに設定
	std::uniform_real_distribution<float> distScale(particleAttributes_.scaleRange.min, particleAttributes_.scaleRange.max);
	//回転をランダムに設定
	std::uniform_real_distribution<float> distRotate(particleAttributes_.rotateRange.min, particleAttributes_.rotateRange.max);
	//位置をランダムに設定
	std::uniform_real_distribution<float> distPosition(particleAttributes_.positionRange.min, particleAttributes_.positionRange.max);
	//速度をランダムに設定
	std::uniform_real_distribution<float> distVelocity(particleAttributes_.velocityRange.min, particleAttributes_.velocityRange.max);
	//色をランダムに設定
	std::uniform_real_distribution<float> distColor(particleAttributes_.colorRange.min, particleAttributes_.colorRange.max);
	//寿命をランダムに設定
	std::uniform_real_distribution<float> distTime(particleAttributes_.lifetimeRange.min, particleAttributes_.lifetimeRange.max);

	Particle particle;
	particle.transforms_.scale = { particleAttributes_.scale.x,distScale(randomEngine),particleAttributes_.scale.z};
	particle.transforms_.rotate = { 0.0f,0.0f,distRotate(randomEngine)};

	Vector3 randomTranslate = { distPosition(randomEngine),distPosition(randomEngine),distPosition(randomEngine) };
	particle.transforms_.translate = translate + randomTranslate;
	particle.velocity_ = { distVelocity(randomEngine),distVelocity(randomEngine),distVelocity(randomEngine) };
	if (particleAttributes_.editColor) {
		particle.color_ = {
			particleAttributes_.color.x,
			particleAttributes_.color.y,
			particleAttributes_.color.z,
			1.0f };
	} else {
		particle.color_ = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	}
	
	//リングの生成
	TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(2.0f,0.1f,particle.transforms_.translate, particle.color_); 

	
	particle.lifeTime_ = distTime(randomEngine);
	particle.currentTime_ = 0.0f;

	return particle;
}

//=============================================================================
// パーティクルの発生
//=============================================================================

std::list<Particle> Particle3d::Emit(const Vector3& emitterPos, uint32_t particleCount) {
	//ランダムエンジン
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	std::list<Particle> particles;
	for (uint32_t index = 0; index < particleCount; ++index) {
		particles.push_back(MakeNewParticle(randomEngine, emitterPos));
	}
	return particles;
}

//=============================================================================
// AABBとの衝突判定
//=============================================================================

bool Particle3d::IsCollision(const AABB& aabb, const Vector3& point) {
	//最近接点を求める
	Vector3 clossestPoint{
		std::clamp(point.x,aabb.min.x,aabb.max.x),
		std::clamp(point.y,aabb.min.y,aabb.max.y),
		std::clamp(point.z,aabb.min.z,aabb.max.z)
	};
	//最近接点と球の中心との距離を求める
	float distance = Vector3Math::Length(clossestPoint - point);
	//距離が半径よりも小さければ衝突
	if (distance <= point.x && distance <= point.y && distance <= point.z) {
		return true;
	}

	return false;
}

//=============================================================================
// パーティクルの配列の結合処理
//=============================================================================

void Particle3d::SpliceParticles(std::list<Particle> particles) {
	particles_.splice(particles_.end(), particles);
}

//=============================================================================
// パーティクルの移動処理
//=============================================================================

void Particle3d::EmitMove(std::list<Particle>::iterator particleIterator) {
	//particle1つの位置更新
	(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
	(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新

}

//=============================================================================
// モデルの設定
//=============================================================================

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}