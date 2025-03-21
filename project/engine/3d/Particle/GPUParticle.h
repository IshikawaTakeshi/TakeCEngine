#pragma once
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "camera/Camera.h"
#include "math/Matrix4x4.h"
#include "3d/Model.h"
#include "3d/Particle/ParticleCommon.h"
#include "ResourceDataStructure.h"
#include <memory>

struct ParticleForCS {
	Vector3 translate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

class GPUParticle {
public:

	GPUParticle() = default;
	~GPUParticle();

	void Initialize(ParticleCommon* particleCommon,  const std::string& filePath);

	void Update();

	void Draw();

	void DisPatchInitializeParticle();

private:

	ParticleCommon* particleCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	ParticleForCS* particleData_ = nullptr;
	uint32_t particleUavIndex_ = 0;
	uint32_t particleSrvIndex_ = 0;

	//PerViewData
	PerView* perViewData_ = nullptr;

	static const uint32_t kNumMaxInstance_ = 1024;

	ComPtr<ID3D12Resource> particleUavResource_ = nullptr;
	ComPtr<ID3D12Resource> perViewResource_ = nullptr;
};