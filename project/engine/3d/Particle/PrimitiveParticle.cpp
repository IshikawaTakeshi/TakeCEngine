#include "PrimitiveParticle.h"
#include "base/DirectXCommon.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/TextureManager.h"
#include "base/TakeCFrameWork.h"
#include "math/MatrixMath.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"
#include "camera/CameraManager.h"
#include "3d/Particle/ParticleCommon.h"
#include <numbers>

//=============================================================================
// コンストラクタ・デストラクタ
//=============================================================================
PrimitiveParticle::PrimitiveParticle(PrimitiveType type) {
	particlePreset_.primitiveType = type;
}


//=============================================================================
// 初期化
//=============================================================================
void PrimitiveParticle::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//ParticleResource生成
	particleResource_ =
		TakeC::DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);
	particleResource_->SetName(L"PrimitiveParticle::particleResource_");
	//perViewResource生成
	perViewResource_ = TakeC::DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
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
	//perViewData初期化
	perViewData_->viewProjection = MatrixMath::MakeIdentity4x4();
	perViewData_->billboardMatrix = MatrixMath::MakeIdentity4x4();

	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	if (particlePreset_.primitiveType == PRIMITIVE_RING) {
		//プリミティブの初期化
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(1.0f, 0.5f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_PLANE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(1.0f, 1.0f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_SPHERE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateSphere(1.0f, filePath);
	} else if(particlePreset_.primitiveType == PRIMITIVE_CONE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateCone(1.0f, 5.0f, 16, filePath);
	} else {
		assert(0 && "未対応の PrimitiveType が指定されました");
	}

	//テクスチャファイルパスの設定
	particlePreset_.textureFilePath = filePath;
	//1フレームの時間取得
	kDeltaTime_ = TakeCFrameWork::GetDeltaTime();
}

//=============================================================================
// 更新処理
//=============================================================================
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
			UpdateMovement(particleIterator);
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

			

			++numInstance_; // 次のインスタンスに進める  
		}
		++particleIterator; // 次のイテレータに進める  
	}

	if (!pendingParticles_.empty()) {
		// 末尾にまとめて追加
		particles_.insert(particles_.end(), pendingParticles_.begin(), pendingParticles_.end());
		pendingParticles_.clear();
	}

	// データをGPUに転送  
	perViewData_->isBillboard = particlePreset_.attribute.isBillboard;
	perViewData_->viewProjection = TakeC::CameraManager::GetInstance().GetActiveCamera()->GetViewProjectionMatrix();
	perViewData_->billboardMatrix = TakeC::CameraManager::GetInstance().GetActiveCamera()->GetRotationMatrix();
}

//=============================================================================
// ImGui更新処理
//=============================================================================
void PrimitiveParticle::UpdateImGui() {
#ifdef _DEBUG
	ParticleAttributes& attributes = particlePreset_.attribute;

	ImGui::Begin("Particle");
	ImGui::Text("Instance Count : %d", numInstance_);
	ImGui::DragFloat3("RingScale", &attributes.scale.x, 0.01f);
	ImGui::DragFloat2("RotateRange", &attributes.rotateRange.min, 0.01f);
	ImGui::DragFloat2("PositionRange", &attributes.positionRange.min, 0.01f);
	ImGui::DragFloat2("VelocityRange", &attributes.velocityRange.min, 0.01f);
	ImGui::DragFloat2("ColorRange", &attributes.colorRange.min, 0.01f);
	ImGui::Checkbox("isBillboard", &attributes.isBillboard);
	ImGui::End();
#endif // _DEBUG
}

//=============================================================================
// 描画処理
//=============================================================================
void PrimitiveParticle::Draw() {

	//BaseParticleGroupの描画処理
	BaseParticleGroup::Draw();
	//プリミティブの描画
	TakeCFrameWork::GetPrimitiveDrawer()->DrawParticle(
		particleCommon_->GetGraphicPSO(BlendState::NORMAL), numInstance_, particlePreset_.primitiveType, primitiveHandle_);
}

//=============================================================================
// パーティクルの生成
//=============================================================================
Particle PrimitiveParticle::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate,const Vector3& direction) {

	return BaseParticleGroup::MakeNewParticle(randomEngine, translate,direction);
}

//=============================================================================
// パーティクルの発生
//=============================================================================
std::list<Particle> PrimitiveParticle::Emit(const Vector3& emitterPos,const Vector3& direction, uint32_t particleCount) {

	return BaseParticleGroup::Emit(emitterPos,direction, particleCount);
}

//=============================================================================
// パーティクルの配列の結合
//=============================================================================
void PrimitiveParticle::SpliceParticles(std::list<Particle> particles) {

	BaseParticleGroup::SpliceParticles(particles);
}

//=============================================================================
// パーティクルのプリセット設定
//=============================================================================
void PrimitiveParticle::SetPreset(const ParticlePreset& preset) {
	particlePreset_ = preset;
	//テクスチャファイルパスの設定
	if (particlePreset_.primitiveType == PRIMITIVE_RING) {
		auto& primitiveMaterial = TakeCFrameWork::GetPrimitiveDrawer()->GetRingData(primitiveHandle_)->material;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	}else if (particlePreset_.primitiveType == PRIMITIVE_PLANE) {
		auto& primitiveMaterial =  TakeCFrameWork::GetPrimitiveDrawer()->GetPlaneData(primitiveHandle_)->material;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_SPHERE) {
		auto& primitiveMaterial =  TakeCFrameWork::GetPrimitiveDrawer()->GetSphereData(primitiveHandle_)->material;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	} else if(particlePreset_.primitiveType == PRIMITIVE_CONE) {
		auto& primitiveMaterial = TakeCFrameWork::GetPrimitiveDrawer()->GetConeData(primitiveHandle_)->material_;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	} else {
		assert(0 && "未対応の PrimitiveType が指定されました");
	}
}

void PrimitiveParticle::EraseParticle() {

	// 全パーティクルの削除
	particles_.clear();
}

//=============================================================================
// パーティクル1つの位置更新
//=============================================================================
void PrimitiveParticle::UpdateMovement(std::list<Particle>::iterator particleIterator) {

	//particle1つの位置更新
	ParticleAttributes& attributes = particlePreset_.attribute;

	// 前フレームの位置を保存
	Vector3 oldPosition = (*particleIterator).transforms_.translate;
	float oldTime = (*particleIterator).currentTime_;

	if (attributes.isTranslate) {
		if (attributes.enableFollowEmitter) {
			//エミッターに追従する場合
			(*particleIterator).transforms_.translate = emitterPos_;
			//(*particleIterator).transforms_.rotate = emitter
		} else {
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;

		}

		if(attributes.isDirectional){
			//方向に沿って移動
			(*particleIterator).velocity_ += attributes.direction * kDeltaTime_;
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;
			
		}
	}

	if (attributes.scaleSetting == static_cast<uint32_t>(ScaleSetting::ScaleUp)) {
		//スケールの更新(拡大)
		(*particleIterator).transforms_.scale.x = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

		(*particleIterator).transforms_.scale.y = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.z = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

	} else if (attributes.scaleSetting == static_cast<uint32_t>(ScaleSetting::ScaleDown)) {
		//スケールの更新(縮小)
		(*particleIterator).transforms_.scale.x = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.y = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.z = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
	}


	(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新
	float newTime = (*particleIterator).currentTime_;

	// --- トレイル（補間）処理 ---
	if (attributes.isParticleTrail && (*particleIterator).isTrailParent_) {

		// タイマーを更新（残りを保持するために減算方式）
		(*particleIterator).trailSpawnTimer_ += kDeltaTime_;

		// もし interval を複数回超えていたら、複数回分生成できるようにループする
		while ((*particleIterator).trailSpawnTimer_ >= attributes.trailEmitInterval) {
			// 残り時間を減らす（0 にリセットするより正確）
			(*particleIterator).trailSpawnTimer_ -= attributes.trailEmitInterval;

			// 指定回数分、位置を補間して子パーティクルを生成
			size_t n = attributes.particlesPerInterpolation;
			for (size_t i = 0; i < n; ++i) {
				float t = static_cast<float>(i + 1) / static_cast<float>(n + 1);

				Particle trailParticle = *particleIterator; // 親をコピー
				// 補間位置
				trailParticle.transforms_.translate =
					Easing::Lerp(oldPosition, (*particleIterator).transforms_.translate, t);
				// 補間経過時間（親の oldTime -> newTime）
				trailParticle.currentTime_ = Easing::Lerp(oldTime, newTime, t);

				//移動させる必要がないため、速度を0に設定
				trailParticle.velocity_ = Vector3(0.0f, 0.0f, 0.0f);

				// トレイルは親化しない（更なるトレイルを作らない）
				trailParticle.isTrailParent_ = false;
				// トレイル固有のタイマーはリセット
				trailParticle.trailSpawnTimer_ = 0.0f;

				// 直接 particles_ に挿入せず pending に積む（次フレーム扱いにする）
				pendingParticles_.push_back(std::move(trailParticle));
			}
		}
	}
}