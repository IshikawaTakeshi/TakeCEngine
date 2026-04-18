#pragma once
#include <json.hpp>

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector4, x, y, z, w)
