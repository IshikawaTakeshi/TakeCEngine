#pragma once
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "camera/Camera.h"
#include "camera/PerView.h"
#include "math/Matrix4x4.h"
#include "3d/Model.h"
#include "3d/Particle/ParticleCommon.h"
#include "ResourceDataStructure.h"
#include "3d/Particle/ParticleAttribute.h"
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

	void DisPatchUpdateParticle();

	uint32_t GetParticleUavIndex() const { return particleUavIndex_; }

	uint32_t GetFreeListIndexUavIndex() const { return freeListIndexUavIndex_; }

	uint32_t GetFreeListUavIndex() const { return freeListUavIndex_; }

	uint32_t GetAttributeSrvIndex() const { return attributeSrvIndex_; }

	ID3D12Resource* GetParticleUavResource() const { return particleUavResource_.Get(); }

	void SetPreset(const ParticlePreset& preset) {
		particlePreset_ = preset;
	}

private:

	ParticleCommon* particleCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;

	ParticleForCS* particleData_ = nullptr;
	uint32_t particleUavIndex_ = 0;
	uint32_t freeListIndexUavIndex_ = 0;
	uint32_t freeListUavIndex_ = 0;

	//PerViewData
	PerView* perViewData_ = nullptr;
	//PerFrameData
	PerFrame* perFrameData_ = nullptr;
	//PerticlePreset
	ParticlePreset particlePreset_;
	ParticleAttributes* particleAttributes_;
	uint32_t attributeSrvIndex_ = 0;
	uint32_t primitiveHandle_ = 0;

	static const uint32_t kNumMaxInstance_ = 1024000;

	ComPtr<ID3D12Resource> particleUavResource_ = nullptr;
	ComPtr<ID3D12Resource> perViewResource_ = nullptr;
	ComPtr<ID3D12Resource> freeListResource_ = nullptr;
	ComPtr<ID3D12Resource> freeListIndexResource_ = nullptr;
	ComPtr<ID3D12Resource> perFrameResource_ = nullptr;
	ComPtr<ID3D12Resource> attributeResource_ = nullptr;
};