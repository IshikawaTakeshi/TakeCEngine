#pragma once
#include "../Vector4.h"
#include <cstdint>

struct Material {
	Vector4 color;
	uint32_t enableLighting;
};