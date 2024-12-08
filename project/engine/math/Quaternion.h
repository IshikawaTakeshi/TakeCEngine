#pragma once
#include "Vector3.h"

struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

namespace QuaternionMath {
	//クォータニオン掛け算
	Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
	//単位クォータニオン
	Quaternion IdentityQuaternion();
	//共役
	Quaternion Conjugate(const Quaternion& q);
	//ノルム
	float Norm(const Quaternion& q);
	//正規化
	Quaternion Normalize(const Quaternion& q);
	//逆クォータニオン
	Quaternion Inverse(const Quaternion& q);
	//任意軸回転クォータニオン
	Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

	//Quaternionの回転結果をベクトルで返す
	Vector3 RotateVector(const Vector3& vector,const Quaternion& quaternion);
}