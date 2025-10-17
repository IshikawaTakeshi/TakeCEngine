#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"

//DirectionalLightのデータ
struct DirectionalLightData {
	Vector4 color_; //ライトの色
	Vector3 direction_; //ライトの向き
	float intensity_; //輝度
};
