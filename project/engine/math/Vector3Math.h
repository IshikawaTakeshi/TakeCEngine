#pragma once
#include "Vector3.h"

struct Quaternion;
//============================================================================
// Vector3Math namespace
//============================================================================
namespace Vector3Math {

	//加算
	Vector3 Add(const Vector3& v1, const Vector3& v2);

	//減算
	Vector3 Subtract(const Vector3& v1, const Vector3& v2);

	//乗算
	Vector3 Multiply(float scalar, const Vector3& v);

	//乗算
	Vector3 Multiply(const Vector3& v1, const Vector3& v2);

	//商算
	Vector3 Divide(float scalar, const Vector3& v);

	//内積
	float Dot(const Vector3& v1, const Vector3& v2);

	//クロス積
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	//長さ(ノルム)
	float Length(const Vector3& v);

	//長さの二乗
	float LengthSq(const Vector3& v);

	//正規化
	Vector3 Normalize(const Vector3& v);

	Vector3 ApplyYawPitch(const Vector3& baseDir, float yawDeg, float pitchDeg);

	Quaternion ToQuaternion(const Vector3& eulerAngles);
};
