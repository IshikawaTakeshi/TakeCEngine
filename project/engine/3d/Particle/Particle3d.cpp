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
#include "Input.h"
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
	
	//instancing用のResource生成
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
	accelerationField_.aabb_.min = { -3.0f,-3.0f,-3.0f };
	accelerationField_.aabb_.max =  { 3.0f, 3.0f, 3.0f };

	//Mapping
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
			EmitMove(particleIterator);
			//EmitMove(particleIterator);
			float alpha = 1.0f - ((*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

			//行列の更新
			worldMatrix_ = MatrixMath::MakeAffineMatrix(
				(*particleIterator).transforms_.scale,
				(*particleIterator).transforms_.rotate,
				(*particleIterator).transforms_.translate
			);

			//ビルボードの処理
			if (particleAttributes_.isBillboard) {
				Matrix4x4 translateMatrix = MatrixMath::MakeTranslateMatrix((*particleIterator).transforms_.translate);
				Matrix4x4 scaleMatrix = MatrixMath::MakeScaleMatrix((*particleIterator).transforms_.scale);
				worldMatrix_ = translateMatrix * CameraManager::GetInstance()->GetActiveCamera()->GetRotationMatrix() * scaleMatrix;
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
			instancingData_[numInstance_].WVP = model_->GetModelData()->rootNode.localMatrix * WVPMatrix_;
			instancingData_[numInstance_].World = model_->GetModelData()->rootNode.localMatrix * worldMatrix_;
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
#ifdef _DEBUG
	ImGui::Text("Particle3d");
	ImGui::Text("ParticleCount:%d", numInstance_);
	particleCommon_->GetGraphicPSO()->UpdateImGui();
	ImGui::Separator();
	ImGui::Text("ParticleAttributes");
	if(ImGui::Button("SoapBubble")) {
		SetAttributesSoapBubble();
	}else if(ImGui::Button("Fire")) {
		SetAttributesFire();
	}
	ImGui::DragFloat3("Scale", &particleAttributes_.scale.x, 0.01f);
	ImGui::DragFloat2("PositionRange", &particleAttributes_.positionRange.min, 0.01f);
	ImGui::DragFloat2("VelocityRange", &particleAttributes_.velocityRange.min, 0.01f);
	ImGui::DragFloat2("ColorRange", &particleAttributes_.colorRange.min, 0.01f);
	ImGui::DragFloat2("LifetimeRange", &particleAttributes_.lifetimeRange.min, 0.01f);
	ImGui::Checkbox("Billboard", &particleAttributes_.isBillboard);
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

	//instancing用のDataを読むためにStructuredBufferのSRVを設定する
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(1, instancingSrvIndex_);

	model_->DrawForParticle(numInstance_);
}

//=============================================================================
// パーティクルの生成
//=============================================================================

Particle Particle3d::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {

	//位置をランダムに設定
	std::uniform_real_distribution<float> distPosition(particleAttributes_.positionRange.min, particleAttributes_.positionRange.max);
	//速度をランダムに設定
	std::uniform_real_distribution<float> distVelocity(particleAttributes_.velocityRange.min, particleAttributes_.velocityRange.max);
	//色をランダムに設定
	std::uniform_real_distribution<float> distColor(particleAttributes_.colorRange.min, particleAttributes_.colorRange.max);
	//寿命をランダムに設定
	std::uniform_real_distribution<float> distTime(particleAttributes_.lifetimeRange.min, particleAttributes_.lifetimeRange.max);

	Particle particle;
	particle.transforms_.scale = { particleAttributes_.scale.x,particleAttributes_.scale.y,particleAttributes_.scale.z };
	particle.transforms_.rotate = { 0.0f,0.0f,0.0f };

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
	particle.lifeTime_ = distTime(randomEngine);
	particle.currentTime_ = 0.0f;
	return particle;
}

//=============================================================================
// パーティクルの発生
//=============================================================================

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
	particles_.splice(particles_.end(),particles);
}

//=============================================================================
// パーティクルの移動処理
//=============================================================================

void Particle3d::EmitMove(std::list<Particle>::iterator particleIterator) {
	//particle1つの位置更新
	(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
	(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新
	
}

//void Particle3d::ConvergenceMove(std::list<Particle>::iterator particleIterator) {
//    // マウスの座標を取得する
//    Vector2 mousePosition = Input::GetInstance()->GetCursorPosition();
//
//    // パーティクルの現在位置とマウスの座標の差を計算する
//	Vector3 direction = Vector3{ mousePosition.x,mousePosition.y,0.0f } - (*particleIterator).transforms_.translate;
//
//    // パーティクルをマウスの座標に収束させる移動量を計算する
//    Vector3 moveAmount = direction * kDeltaTime_;
//
//    // パーティクルの位置を更新する
//    (*particleIterator).transforms_.translate += moveAmount;
//}

//=============================================================================
// パーティクルの属性を設定(シャボン玉)
//=============================================================================

void Particle3d::SetAttributesSoapBubble() {
	particleAttributes_.scale = { 1.0f,1.0f,1.0f };
	particleAttributes_.positionRange = { -2.0f,2.0f };
	particleAttributes_.velocityRange = { -7.1f,3.1f };
	particleAttributes_.colorRange = { 0.0f,1.0f };
	particleAttributes_.lifetimeRange = { 1.0f,2.0f };
	particleAttributes_.isBillboard = false;
	particleAttributes_.editColor = true;
	particleAttributes_.color = { 0.1f,0.8f,0.9f };
}

//=============================================================================
// パーティクルの属性を設定(松明の火)
//=============================================================================

void Particle3d::SetAttributesFire() {
	particleAttributes_.scale = { 1.0f,1.0f,1.0f };
	particleAttributes_.positionRange = { -0.1f,1.0f };
	particleAttributes_.velocityRange = { -1.1f,1.1f };
	particleAttributes_.colorRange = { 0.0f,1.0f };
	particleAttributes_.lifetimeRange = { 1.0f,3.0f };
	particleAttributes_.isBillboard = true;
	particleAttributes_.editColor = true;
	particleAttributes_.color = { 1.0f,0.14f,0.0f };
	accelerationField_.acceleration_ = { 0.0f,3.7f,0.0f };
	accelerationField_.aabb_.min = { -10.3f,-10.3f,-10.3f };
	accelerationField_.aabb_.max = { 10.3f,10.3f,10.3f };
	accelerationField_.position_ = { 4.0f,0.0f,0.0f };
}

void Particle3d::SetAttributesHadouken() {
	particleAttributes_.scale = { 1.0f,1.0f,1.0f };
	particleAttributes_.positionRange = { -0.1f,1.0f };
	particleAttributes_.velocityRange = { -1.1f,1.1f };
	particleAttributes_.colorRange = { 0.0f,1.0f };
	particleAttributes_.lifetimeRange = { 0.1f,0.3f };
	particleAttributes_.isBillboard = true;
	particleAttributes_.editColor = true;
	particleAttributes_.color = { 0.0f,0.9f,0.2f };
	accelerationField_.acceleration_ = { 0.0f,3.7f,0.0f };
	accelerationField_.aabb_.min = { -10.3f,-10.3f,-10.3f };
	accelerationField_.aabb_.max = { 10.3f,10.3f,10.3f };
	accelerationField_.position_ = { 4.0f,0.0f,0.0f };
}

//=============================================================================
// モデルの設定
//=============================================================================

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

//void Particle3d::SetModelTexture(const std::string& texturefilePath) {
//	//model_->GetMesh()->GetMaterial()->
//}
