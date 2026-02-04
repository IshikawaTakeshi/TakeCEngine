#include "ParticleEmitter.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/SrvManager.h"
#include "engine/math/Easing.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/MatrixMath.h"
#include "engine/Utility/JsonLoader.h"
#include "3d/Particle/GPUParticle.h"
#include "2d/WireFrame.h"


//================================================================================================
// 初期化
//================================================================================================

void ParticleEmitter::Initialize(const std::string& emitterName, const std::string& presetInfo) {
	//Emitter初期化
	emitterName_ = emitterName;


	//prest情報の取得
	preset_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>(presetInfo);

	// ParticleManagerのアロケーターからエミッターIDを取得して登録
	emitterID_ = TakeCFrameWork::GetParticleManager()->EmitterAllocate(this);

	particleName_ = preset_.presetName;
	emitDirection_ = { 0.0f,0.0f,1.0f };
	particleCount_ = preset_.attribute.emitCount; // 発生させるパーティクルの数
	frequency_ = preset_.attribute.frequency; // 発生頻度
	maxInterpolationCount_ = preset_.attribute.particlesPerInterpolation; // 最大補間回数
	frequencyTime_ = 0.0f;
	isEmit_ = false;
}

//==================================================================================
// 更新処理
//==================================================================================

void ParticleEmitter::Update() {

	//transform.rotateによってDirectionを更新
	Matrix4x4 rotateMatrix = MatrixMath::MakeRotateMatrix(transforms_.rotate);
	emitDirection_ = MatrixMath::Transform({ 0.0f,0.0f,1.0f }, rotateMatrix);

	//エミッターの更新
	if (!isEmit_) return;
	frequencyTime_ += TakeCFrameWork::GetDeltaTime();
	if (frequency_ <= frequencyTime_) {

		if (TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.isEmitterTrail == true) {
			// 移動ベクトルと距離
			Vector3 moveVector = transforms_.translate - prevTranslate_;
			float moveDistance = Vector3Math::Length(moveVector);

			// 移動距離に応じた補間回数を計算
			// 例: 1. 0単位ごとに1回補間
			const float interpolationUnit = 1.0f; // 調整可能
			int interpolationCount = static_cast<int>(moveDistance / interpolationUnit);

			// 最大補間回数の制限（パフォーマンス対策）
			interpolationCount = std::min(interpolationCount, maxInterpolationCount_);

			if (interpolationCount > 0) {
				// 等間隔で補間位置から発生
				for (int i = 0; i < interpolationCount; ++i) {
					float t = static_cast<float>(i + 1) / (interpolationCount + 1);
					Vector3 interpolatedPos = prevTranslate_ + moveVector * t;
					// エミッターIDを含めてパーティクルを発生
					TakeCFrameWork::GetParticleManager()->EmitWithEmitter(
						emitterID_,
						particleName_,
						interpolatedPos,
						emitDirection_,
						particleCount_
					);
				}
			}
		}
		// エミッターIDを含めてパーティクルを発生
		TakeCFrameWork::GetParticleManager()->EmitWithEmitter(
			emitterID_,
			particleName_,
			transforms_.translate,
			emitDirection_,
			particleCount_
		);
		frequencyTime_ -= frequency_; //余計に過ぎた時間も加味して頻度計算する
		prevTranslate_ = transforms_. translate;
	}
}

//==================================================================================
// ImGuiの更新
//==================================================================================

void ParticleEmitter::UpdateImGui() {

	ImGui::Begin("ParticleEmitter");
	if (ImGui::TreeNode(emitterName_.c_str())) {
		ImGui::Checkbox("Emit", &isEmit_);
		ImGui::SliderFloat3("Translate", &transforms_.translate.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Rotate", &transforms_.rotate.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Scale", &transforms_.scale.x, 0.0f, 10.0f);
		int count = particleCount_;
		ImGui::SliderInt("ParticleCount", &count, 0, 100);
		particleCount_ = count;
		ImGui::SliderFloat("Frequency", &frequency_, 0.0f, 1.0f);

		ImGui::TreePop();
	}
	ImGui::End();
}

//==================================================================================
// 描画処理
//==================================================================================

void ParticleEmitter::DrawWireFrame() {

	TakeCFrameWork::GetWireFrame()->DrawSphere(transforms_.translate, 0.3f, { 1.0f, 0.0f, 1.0f, 1.0f });
}

//==================================================================================
// パーティクルの発生
//==================================================================================

void ParticleEmitter::Emit() {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, transforms_.translate,emitDirection_, particleCount_);
}

void ParticleEmitter::Emit(const Vector3& position) {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, position,emitDirection_, particleCount_);
}

//==================================================================================
// パーティクル名の設定
//==================================================================================

void ParticleEmitter::SetParticleName(const std::string& particleName) {
	particleName_ = particleName;
	frequency_ = TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.frequency;
	particleCount_ = TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.emitCount;
}