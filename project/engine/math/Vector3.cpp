#include "Vector3.h"
#include "Vector3Math.h"
#include "Utility/Logger.h"

//===============================================================
//		演算子オーバーロード
//===============================================================

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return Vector3Math::Add(v1, v2);
}

Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return Vector3Math::Subtract(v1, v2);
}

Vector3 operator*(float s, const Vector3& v) {
	return Vector3Math::Multiply(s, v);
}

Vector3 operator*(const Vector3& v, float s) {
	return s * v;
}

Vector3 operator*(const Vector3& v1, const Vector3& v2) {

	return Vector3Math::Multiply(v1, v2);
}

Vector3 operator/(float s, const Vector3& v) {
	return Vector3Math::Multiply(1.0f / s, v);
}

Vector3 operator/(const Vector3& v, float s) {
	return s / v;
}

Vector3 Vector3::operator+() const {
	return Vector3(x, y, z);
}

Vector3 Vector3::operator-() const {
	return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector3 Vector3::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3 Vector3::operator*=(float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vector3 Vector3::operator/=(float s) {
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

bool operator==(const Vector3& lhs, const Vector3& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator!=(const Vector3& lhs, const Vector3& rhs) {
	return !(lhs == rhs);
}

bool operator<(const Vector3& lhs, const Vector3& rhs) {
	return (lhs.x < rhs.x) && (lhs.y < rhs.y) && (lhs.z < rhs.z);
}

bool operator<=(const Vector3& lhs, const Vector3& rhs) {
	return (lhs.x <= rhs.x) && (lhs.y <= rhs.y) && (lhs.z <= rhs.z);
}

bool operator>(const Vector3& lhs, const Vector3& rhs) {
	return (rhs < lhs);
}

bool operator>=(const Vector3& lhs, const Vector3& rhs) {
	return (rhs <= lhs);
}

float Vector3::Dot(const Vector3& other) const {
	return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::Cross(const Vector3& other) const {
	return Vector3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

float Vector3::Length() const {
	return std::sqrt(x * x + y * y + z * z);
}

Vector3 Vector3::Normalize() const {
	float len = Length();
	return (len > 0) ? (*this * (1.0f / len)) : *this;
}