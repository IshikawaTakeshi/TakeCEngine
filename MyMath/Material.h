#pragma once
#include "../Vector4.h"
#include <cstdint>

struct Material {
	Vector4 color; //カラー
	uint32_t enableLighting; //ライティングを有効にするフラグ
};