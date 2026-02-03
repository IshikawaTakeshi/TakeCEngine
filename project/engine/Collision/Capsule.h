#pragma once
#include "engine/math/Vector3.h"

struct Capsule {
	Vector3 start; // カプセルの一端の位置
	Vector3 end; // カプセルのもう一端の位置
	float radius;   // カプセルの半径
};