#pragma once
#include "../MyMath/Matrix4x4.h"

struct TransformMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};