#pragma once
#include "engine/math/Matrix4x4.h"

//View情報
struct PerView {
	Matrix4x4 viewProjection;
	Matrix4x4 billboardMatrix;
	bool isBillboard;
};