#include "Vector3.h"
#include "Vector3Math.h"


//===============================================================
//		二項演算子
//===============================================================

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return Vector3Math::Add(v1, v2);
}

Vector3 operator+(const Vector3& v) {
	return v;
}

Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return Vector3Math::Subtract(v1, v2);
}

Vector3 operator-(const Vector3& v) {
	return { -v.x, -v.y, -v.z };
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


//===============================================================
//		複合代入演算子
//===============================================================

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

bool Vector3::operator==(const Vector3& v) const {
	return x == v.x && y == v.y && z == v.z;
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