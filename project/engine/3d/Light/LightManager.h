#pragma once
#include "engine/3d/Light/DirectionalLight.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Light/SpotLight.h"
#include "engine/3d/Light/LightCounter.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

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
	//--------- accessor -----------------------------------------------------
	

private:

	//========================================================================
	//	private variables
	//========================================================================

	LightCounter<PointLightData> pointLightCounter_;
	LightCounter<SpotLightData> spotLightCounter_;

	DirectionalLightData* dirLightData_ = nullptr;
	PointLghtData* pointLightData_ = nullptr;
	pointLightData_* spotLightData_ = nullptr;

};