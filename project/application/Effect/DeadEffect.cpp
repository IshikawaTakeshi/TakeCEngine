#include "DeadEffect.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"
#include <algorithm>

//===================================================================================
//　初期化
//===================================================================================
void DeadEffect::Initialize() {
	//パーティクルエミッターの生成
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("explosion", "DeadExplosionEffect.json");
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("smoke", "DeadSmokeEffect.json");
	particleEmitter_[1]->SetRotate({ -1.4f,0.0f,0.0f });
	particleEmitter_[1]->SetParticleName("DeadSmokeEffect");
	particleEmitter_[2] = std::make_unique<ParticleEmitter>(); 
	particleEmitter_[2]->Initialize("DamageSpark2", "DamageSpark2.json");

	//PointLightの設定
	pointLightData_.enabled_ = 0;
	pointLightData_.color_ = { 1.0f,0.5f,0.1f,1.0f };
	pointLightData_.intensity_ = 0.0f;
	pointLightData_.radius_ = 20.0f;
	pointLightData_.decay_ = 1.0f;
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);
}

//===================================================================================
//　更新
//===================================================================================
void DeadEffect::Update(const Vector3& translate) {
	//タイマー更新
	timer_.Update();

	if (timer_.IsFinished()) {
		//タイマーが終了したらパーティクル発生停止
		particleEmitter_[0]->SetIsEmit(false);
	}
	
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DeadExplosionEffect")->SetEmitterPosition(translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DeadSmokeEffect")->SetEmitterPosition(translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark2")->SetEmitterPosition(translate);

	//パーティクルエミッター更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(translate);
		emitter->Update();
	}

	//ポイントライトの更新
	pointLightData_.position_ = translate + Vector3{ 0.0f,20.0f,0.0f };
	pointLightData_.intensity_ = 120.0f * (1.0f - timer_.GetEase(Easing::EasingType::OUT_QUAD));
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);
}

//===================================================================================
//　開始
//===================================================================================
void DeadEffect::Start() {
	timer_.Initialize(1.0f, 0.0f); //タイマー初期化
	for (auto& emitter : particleEmitter_) {
		emitter->SetIsEmit(true); //パーティクル発生開始
	}
	pointLightData_.enabled_ = 1; //ライト有効化
}