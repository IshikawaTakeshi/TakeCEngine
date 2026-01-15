#include "DeadEffect.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"
#include <algorithm>

//===================================================================================
//　初期化
//===================================================================================
void DeadEffect::Initialize() {
	//パーティクルエミッターの生成
	explosionParticleEmitter_ = std::make_unique<ParticleEmitter>();
	explosionParticleEmitter_->Initialize("explosion", {}, 6, 0.5f);
	explosionParticleEmitter_->SetParticleName("DeadExplosionEffect");
	smokeParticleEmitter_ = std::make_unique<ParticleEmitter>();
	smokeParticleEmitter_->Initialize("smoke", {}, 20, 0.1f);
	smokeParticleEmitter_->SetRotate({ -1.4f,0.0f,0.0f });
	smokeParticleEmitter_->SetParticleName("DeadSmokeEffect");

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
		explosionParticleEmitter_->SetIsEmit(false);
	}
	
	explosionParticleEmitter_->SetTranslate(translate);
	smokeParticleEmitter_->SetTranslate(translate);

	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DeadExplosionEffect")->SetEmitterPosition(translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DeadSmokeEffect")->SetEmitterPosition(translate);
	//パーティクルエミッター更新
	explosionParticleEmitter_->Update();
	smokeParticleEmitter_->Update();

	//ポイントライトの更新
	pointLightData_.position_ = translate + Vector3{ 0.0f,20.0f,0.0f };
	pointLightData_.intensity_ = 120.0f * (1.0f - timer_.GetEase(Easing::EasingType::OUT_QUAD));
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);
}

//===================================================================================
//　開始
//===================================================================================
void DeadEffect::Start() {
	timer_.Initialize(0.01f, 0.0f); //タイマー初期化
	explosionParticleEmitter_->SetIsEmit(true); //パーティクル発生開始
	smokeParticleEmitter_->SetIsEmit(true);
	pointLightData_.enabled_ = 1; //ライト有効化
}