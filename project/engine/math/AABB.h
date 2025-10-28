#pragma once
#include "Vector3.h"

//============================================================================
// AABB struct
//============================================================================

// Axis-Aligned Bounding Box
struct AABB {
	Vector3 min; // 最小座標
	Vector3 max; // 最大座標
};