#pragma once
#include "engine/math/Matrix4x4.h"
#include "engine/math/Vector3.h"
struct LightCamera {
	Matrix4x4 viewProjection_;
	Vector3 position_;
	float nearClip_ = 0.1f;
	float farClip_ = 3800.0f;

	void Initialize(const Vector3& positon);
	void Update();
};

