#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/matrix4x4.h"

//============================================================================
// CameraForGPU struct
//============================================================================

//定数バッファ用の構造体
struct CameraForGPU {
	Matrix4x4 ProjectionInverse; // 投影行列の逆行列
	Vector3 worldPosition;
	float padding;
};