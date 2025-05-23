#pragma once
#include "3d/Particle/BaseParticleGroup.h"
#include "Primitive/PrimitiveDrawer.h"


class PrimitiveParticle : public BaseParticleGroup {

public:
	PrimitiveParticle(PrimitiveType type);
	~PrimitiveParticle();

	void Initialize(ParticleCommon* particleCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos, uint32_t particleCount);

	void SpliceParticles(std::list<Particle> particles);

private:

	PrimitiveType type_ = PRIMITIVE_RING;
};

