#pragma once
#include "ResourceDataStructure.h"
#include "Particle3d.h"
#include <list>
#include <wrl.h>

class DirectXCommon;
class ParticleManager {

public:
	ParticleManager();
	~ParticleManager();

	void Initialize();

	void Update();

private:

	struct ParticleGroup {
		ModelMaterialData modelMaterialData;
		std::list<Particle3d*> particles;
		uint32_t instancingSrvIndex;
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		uint32_t instanceCount;
		//インスタンシングデータを書き込むためのポインタ
	};

};

