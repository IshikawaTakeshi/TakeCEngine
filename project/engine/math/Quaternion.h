#pragma once

struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

namespace QuaternionMath {
	Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
	Quaternion IdentityQuaternion();
	Quaternion Conjugate(const Quaternion& q);
	float Norm(const Quaternion& q);
	Quaternion Normalize(const Quaternion& q);
	Quaternion Inverse(const Quaternion& q);
}