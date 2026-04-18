#pragma once
#include "engine/math/Matrix4x4.h"
#include "engine/math/Vector3.h"
//=============================================================================
//	LightCameraInfo struct
//=============================================================================
struct LightCameraInfo {
	Matrix4x4 viewProjection_;
	Vector3 position_;
	float padding_;
};