#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include <cstdint>

struct DirectionalLight {
	//ライトの色
	Vector4 color_;
	//ライトの向き
	Vector3 direction_;
	//輝度
	float intensity_;
};

