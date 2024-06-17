#pragma once
#include "../Vector4.h"
#include "../MyMath/Matrix3x3.h"
#include <cstdint>

struct Material {
	Vector4 color; //カラー
	uint32_t enableLighting; //ライティングを有効にするフラグ
	float padding[3];
	Matrix4x4 uvTransform; //UVトランスフォーム
};