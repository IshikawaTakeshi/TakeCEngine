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
#include <numbers>


Particle3d::~Particle3d() {
	instancingResource_.Reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Particle3d::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//モデルの読み込み
	SetModel(filePath);
	
	//TransformationMatrix用のResource生成
	instancingResource_ = 
		DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);

	//SRVのインデックスを取得
	instancingSrvIndex_ = particleCommon_->GetSrvManager()->Allocate();

	//SRVの生成
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForGPU),
		instancingResource_.Get(),
		instancingSrvIndex_
	);

	//Acceleration初期化
	accelerationField_.acceleration_ = { 0.0f,-9.8f,0.0f };
	accelerationField_.aabb_.min_ = { -3.0f,-3.0f,-3.0f };
	accelerationField_.aabb_.max_ =  { 3.0f, 3.0f, 3.0f };

	//TransformationMatrix用
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	//単位行列を書き込んでおく
	for (uint32_t i = 0; i < kNumMaxInstance_; i++) {
		instancingData_[i].WVP = MatrixMath::MakeIdentity4x4();
		instancingData_[i].World = MatrixMath::MakeIdentity4x4();
		instancingData_[i].Color = { 1.0f,1.0f,1.0f,1.0f };
	}

	//カメラのセット
	camera_ = particleCommon_->GetDefaultCamera();
}

//=============================================================================
// 更新処理
//=============================================================================

void Particle3d::Update() {

	//ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	numInstance_ = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end(); ) {
		
		//MEMO:Particleの総出現数だけ繰り返す
		if (numInstance_ < kNumMaxInstance_) {

			//寿命が来たら削除
			if ((*particleIterator).lifeTime_ <= (*particleIterator).currentTime_) {
				particleIterator = particles_.erase(particleIterator); 
				continue;
			}

			//加速フィールドの位置更新
			if(IsCollision(accelerationField_.aabb_, (*particleIterator).transforms_.translate)) {
				(*particleIterator).velocity_ += accelerationField_.acceleration_ * kDeltaTime_;
			}

			//particle1つの位置更新
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
			(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新
			float alpha = 1.0f - ((*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

			//行列の更新
			worldMatrix_ = MatrixMath::MakeAffineMatrix(
				(*particleIterator).transforms_.scale,
				(*particleIterator).transforms_.rotate,
				(*particleIterator).transforms_.translate
			);

			//ビルボードの処理
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

			//インスタンシングデータをGPUに転送
			instancingData_[numInstance_].WVP = WVPMatrix_;
			instancingData_[numInstance_].World = worldMatrix_;
			instancingData_[numInstance_].Color = (*particleIterator).color_;
			instancingData_[numInstance_].Color.w = alpha;

			++numInstance_; //次のインスタンスに進める
		}
		++particleIterator; //次のイテレータに進める
	}
}

//=============================================================================
// ImGuiの更新
//=============================================================================

void Particle3d::UpdateImGui() {
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	ImGui::Text("Particle3d");
	ImGui::Checkbox("Billboard", &isBillboard_);
	ImGui::Text("ParticleCount:%d", numInstance_);
	particleCommon_->GetPSO()->UpdateImGui();
}

void Particle3d::Draw() {

	//instancing用のDataを読むためにStructuredBufferのSRVを設定する
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(1, instancingSrvIndex_);

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

std::list<Particle> Particle3d::Emit(const Vector3& emitterPos,uint32_t particleCount) {
	//ランダムエンジン
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	std::list<Particle> particles;
	for (uint32_t index = 0; index < particleCount; ++index) {
		particles.push_back(MakeNewParticle(randomEngine,emitterPos));
	}
	return particles;
}

bool Particle3d::IsCollision(const AABB& aabb, const Vector3& point) {
	//最近接点を求める
	Vector3 clossestPoint{
		std::clamp(point.x,aabb.min_.x,aabb.max_.x),
		std::clamp(point.y,aabb.min_.y,aabb.max_.y),
		std::clamp(point.z,aabb.min_.z,aabb.max_.z)
	};
	//最近接点と球の中心との距離を求める
	float distance = Vector3Math::Length(clossestPoint - point);
	//距離が半径よりも小さければ衝突
	if (distance <= point.x && distance <= point.y && distance <= point.z) {
		return true;
	}

	return false;
}

void Particle3d::SpliceParticles(std::list<Particle> particles) {
	particles_.splice(particles_.end(),particles);
}

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}