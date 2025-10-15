#include "DeadEffect.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"
#include <algorithm>

void DeadEffect::Initialize() {
	//パーティクルエミッターの生成
	explosionParticleEmitter_ = std::make_unique<ParticleEmitter>();
	explosionParticleEmitter_->Initialize("explosion", {}, 100, 0.1f);
	explosionParticleEmitter_->SetParticleName("DeadExplosionEffect");
	smokeParticleEmitter_ = std::make_unique<ParticleEmitter>();
	smokeParticleEmitter_->Initialize("smoke", {}, 100, 0.1f);
	smokeParticleEmitter_->SetParticleName("DeadSmokeEffect");
}

void DeadEffect::Update(const Vector3& translate) {
	//タイマー更新
	timer_.Update();


	if (timer_.IsFinished()) {
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

void DeadEffect::Start() {
	timer_.Initialize(2.0f, 0.0f);
	explosionParticleEmitter_->SetIsEmit(true);
	smokeParticleEmitter_->SetIsEmit(true);
}