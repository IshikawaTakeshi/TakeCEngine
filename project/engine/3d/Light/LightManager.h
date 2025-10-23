#pragma once
#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/3d/Light/DirectionalLight.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Light/SpotLight.h"
#include "engine/3d/Light/LightCounter.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
#include <unordered_map>

//============================================================================
//		LightManager class
//============================================================================

class LightManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================
	LightManager() = default;
	~LightManager() = default;

	void Initialize();
	void Update();

	//--------- accessor -----------------------------------------------------
	

private:

	//========================================================================
	//	private variables
	//========================================================================

	LightCounter<PointLightData> pointLightCounter_;
	LightCounter<SpotLightData> spotLightCounter_;

	DirectionalLightData* dirLightData_ = nullptr;
	PointLightData* pointLightData_ = nullptr;
	SpotLightData* spotLightData_ = nullptr;

	ComPtr<ID3D12Resource> dirLightResource_;
	ComPtr<ID3D12Resource> pointLightResource_;
	ComPtr<ID3D12Resource> spotLightResource_;

	uint32_t pointLightSrvIndex_ = 0;
	uint32_t spotLightSrvIndex_ = 0;

	std::unordered_map<int, std::unique_ptr<PointLightData>> pointLightMap_;
	std::unordered_map<int, std::unique_ptr<SpotLightData>> spotLightMap_;
};