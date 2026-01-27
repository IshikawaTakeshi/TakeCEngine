#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"

//============================================================================
//PointLightのデータ
//============================================================================
struct PointLightData {
	Vector4 color_; //ライトの色
	Vector3 position_; //ライトの位置
	float intensity_; //輝度
	float radius_; //影響範囲
	float decay_; //減衰率
	uint16_t enabled_; //ライトの有効・無効
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	PointLightData,
	color_,
	position_,
	intensity_,
	radius_,
	decay_,
	enabled_)