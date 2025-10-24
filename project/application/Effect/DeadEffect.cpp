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
	smokeParticleEmitter_->Initialize("smoke", {}, 20, 0.3f);
	smokeParticleEmitter_->SetRotate({ -1.4f,0.0f,0.0f });
	smokeParticleEmitter_->SetParticleName("DeadSmokeEffect");
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
}

//===================================================================================
//　開始
//===================================================================================
void DeadEffect::Start() {
	timer_.Initialize(0.01f, 0.0f);
	explosionParticleEmitter_->SetIsEmit(true);
	smokeParticleEmitter_->SetIsEmit(true);
}