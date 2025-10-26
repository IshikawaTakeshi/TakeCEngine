#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"

//============================================================================
//SpotLightのデータ
//============================================================================
struct SpotLightData {
	Vector4 color_;       //ライトの色
	Vector3 position_;    //ライトの位置
	float intensity_;     //輝度
	Vector3 direction_;   //ライトの向き
	float distance_;      //影響範囲
	float decay_;         //減衰率
	float cosAngle_;     //スポットライトの角度
	float penumbraAngle_; //影のぼかし角度
};