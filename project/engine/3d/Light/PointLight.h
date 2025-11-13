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
	float padding[3];
};