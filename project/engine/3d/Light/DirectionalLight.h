#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"
#include "engine/math/Matrix4x4.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <json.hpp>

//============================================================================
//DirectionalLightのデータ
//============================================================================
struct DirectionalLightData {
	Vector4 color_; //ライトの色
	Vector3 direction_; //ライトの向き
	float intensity_; //輝度
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	DirectionalLightData, 
	color_,
	direction_,
	intensity_)

TAKEC_DEFINE_JSON_DIRECTORY_PATH(DirectionalLightData, "Resources/JsonLoader/LightData/");