#pragma once
#include "Vector3.h"
#include <json.hpp>

//============================================================================
// AABB struct
//============================================================================

// Axis-Aligned Bounding Box
struct AABB {
	Vector3 min; // 最小座標
	Vector3 max; // 最大座標

	void Reset();
	void Expand(const Vector3& point);
	Vector3 GetSize() const;
	Vector3 GetCenter() const;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AABB, min, max)