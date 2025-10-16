#pragma once
#include "engine/3d/Particle/ParticleEmitter.h"
#include "engine/Utility/Timer.h"
#include <string>

//============================================================================
// DeadEffect class
//============================================================================
class DeadEffect {
public:

	DeadEffect() = default;
	~DeadEffect() = default;

	//初期化
	void Initialize();
	//更新
	void Update(const Vector3& translate);

	void Start();

private:


	//パーティクルエミッター
	std::unique_ptr<ParticleEmitter> explosionParticleEmitter_ = nullptr;
	std::unique_ptr<ParticleEmitter> smokeParticleEmitter_ = nullptr;
	//タイマー
	Timer timer_;
};

