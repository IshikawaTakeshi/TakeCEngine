#include "BaseParticleGroup.h"
#include "PipelineStateObject.h"
#include "ModelManager.h"
#include "MatrixMath.h"
#include "Vector3Math.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "SrvManager.h"
#include "ModelCommon.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "camera/CameraManager.h"
#include <numbers>

//=============================================================================
// 描画処理
//=============================================================================

void BaseParticleGroup::Draw() {

	// perViewResource
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		particleCommon_->GetGraphicPSO(BlendState::NORMAL)->GetGraphicBindResourceIndex("gPerView"), perViewResource_->GetGPUVirtualAddress());
	// particleResource
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		particleCommon_->GetGraphicPSO(BlendState::NORMAL)->GetGraphicBindResourceIndex("gParticle"), particleSrvIndex_);
}


//=============================================================================
// パーティクルの生成
//=============================================================================
Particle BaseParticleGroup::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate, const Vector3& directoin) {

	ParticleAttributes attributes = particlePreset_.attribute;
	//スケールをランダムに設定
	std::uniform_real_distribution<float> distScale(attributes.scaleRange.min, attributes.scaleRange.max);
	//回転をランダムに設定
	std::uniform_real_distribution<float> distRotate(attributes.rotateRange.min, attributes.rotateRange.max);
	//位置をランダムに設定
	std::uniform_real_distribution<float> distPosition(attributes.positionRange.min, attributes.positionRange.max);
	//速度をランダムに設定
	std::uniform_real_distribution<float> distVelocity(attributes.velocityRange.min, attributes.velocityRange.max);
	//色をランダムに設定
	std::uniform_real_distribution<float> distColor(attributes.colorRange.min, attributes.colorRange.max);
	//寿命をランダムに設定
	std::uniform_real_distribution<float> distTime(attributes.lifetimeRange.min, attributes.lifetimeRange.max);

	Particle particle;
	particle.transforms_.scale = { attributes.scale.x,attributes.scale.y,attributes.scale.z };

	// 回転の設定を条件分岐で明確に分ける
	if (!attributes.isDirectional) {
		// ランダムな回転を設定
		particle.transforms_.rotate = { 0.0f, 0.0f, distRotate(randomEngine) };
	}

	Vector3 randomTranslate = { distPosition(randomEngine),distPosition(randomEngine),distPosition(randomEngine) };
	particle.transforms_.translate = translate + randomTranslate;

	if (attributes.isDirectional) {
		//方向に沿って移動
		attributes.direction = directoin;
		particle.velocity_ = attributes.direction * distVelocity(randomEngine);
	}
	else {
		//ランダムな方向に飛ばす
		particle.velocity_ = { distVelocity(randomEngine),distVelocity(randomEngine),distVelocity(randomEngine) };
	}

	if (attributes.alignRotationToEmitter) {

		Vector3 dir = attributes.direction;
		if (Vector3Math::LengthSq(dir) > 1e-6f) {

			Vector3 to = Vector3Math::Normalize(dir);

			// パーティクルの基準前方向（ローカル +Z を前と仮定）
			Vector3 from = { 0.0f, 0.0f, 1.0f };
			// PrimitiveTypeごとのデフォルトの向きに合わせて基準ベクトルを変更
			switch (particlePreset_.primitiveType) {
			case PRIMITIVE_CONE:
			case PRIMITIVE_CYLINDER:
				from = { 0.0f, 1.0f, 0.0f }; // 円錐・円柱はY軸が高さ方向
				break;
			case PRIMITIVE_PLANE:
				from = { 0.0f, 0.0f, -1.0f }; // Planeは法線が-Zを向いている
				break;
			default:
				from = { 0.0f, 0.0f, 1.0f }; // その他（Ring, Sphere, Cube）はZ軸基準
				break;
			}

			float d = Vector3Math::Dot(from, to);
			d = std::clamp(d, -1.0f, 1.0f);

			Quaternion targetRotate;

			if (d > 1.0f - 1e-5f) {
				targetRotate = QuaternionMath::IdentityQuaternion();
			}
			else if (d < -1.0f + 1e-5f) {
				// 真逆(180度)は軸が不定なので、fromと直交する軸を適当に選ぶ
				Vector3 ortho = (std::fabs(from.y) < 0.999f) ? Vector3{ 0,1,0 } : Vector3{ 1,0,0 };
				Vector3 axis = Vector3Math::Normalize(Vector3Math::Cross(from, ortho));
				targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion(axis, std::numbers::pi_v<float>);
			}
			else {
				Vector3 axis = Vector3Math::Cross(from, to);
				axis = Vector3Math::Normalize(axis);
				float angle = std::acos(d);
				targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion(axis, angle);
			}

			Quaternion& current = particle.transforms_.rotate;

			// shortest-arc
			if (QuaternionMath::Dot(current, targetRotate) < 0.0f) {
				targetRotate = -targetRotate;
			}

			current = Easing::Slerp(current, targetRotate, 0.1f); // 例: 0.05f〜0.3f
			current = QuaternionMath::Normalize(current);
		}

	}


	if (attributes.editColor) {
		//色を編集する場合
		particle.color_ = {
			attributes.color.x,
			attributes.color.y,
			attributes.color.z,
			1.0f };
	}
	else {
		//ランダムな色にする場合
		particle.color_ = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	}

	//寿命の設定
	particle.lifeTimer_.Initialize(distTime(randomEngine), 0.0f);
	//トレイルエフェクトの親フラグ初期化
	particle.isTrailParent_ = true;

	if (attributes.isBillboard == true) {
		//Billboardの場合
		perViewData_->isBillboard = true;
	}

	return particle;
}

Particle BaseParticleGroup::MakeNewParticleWithEmitter(uint32_t emitterID, std::mt19937& randomEngine, const Vector3& translate, const Vector3& direction) {
	Particle particle = MakeNewParticle(randomEngine, translate, direction);
	particle.emitterID_ = emitterID;
	return particle;
}

//=============================================================================
// パーティクルの発生
//=============================================================================
std::list<Particle> BaseParticleGroup::Emit(const Vector3& emitterPos, const Vector3& direction, uint32_t particleCount) {
	//ランダムエンジン
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	std::list<Particle> particles;
	for (uint32_t index = 0; index < particleCount; ++index) {
		particles.push_back(MakeNewParticle(randomEngine, emitterPos, direction));
	}
	return particles;
}

std::list<Particle> BaseParticleGroup::EmitWithEmitter(uint32_t emitterID, const Vector3& emitterPos, const Vector3& direction, uint32_t particleCount) {
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	std::list<Particle> particles;
	for (uint32_t index = 0; index < particleCount; ++index) {
		particles.push_back(MakeNewParticleWithEmitter(emitterID, randomEngine, emitterPos, direction));
	}
	return particles;
}

//=============================================================================
// パーティクルの配列の結合処理
//=============================================================================
void BaseParticleGroup::SpliceParticles(std::list<Particle> particles) {
	particles_.splice(particles_.end(), particles);
}

void BaseParticleGroup::SetPreset(const ParticlePreset& preset) {
	//プリセットの設定
	particlePreset_ = preset;

}

void BaseParticleGroup::SetEmitterPosition(const Vector3& position) {
	//エミッターの位置設定
	emitterPos_ = position;
}

void BaseParticleGroup::SetEmitDirection(const Vector3& direction) {
	//発生方向の設定
	emitDirection_ = direction;
}

