#pragma once
#include <cmath>
#include <algorithm>
#include <json.hpp>

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;

	Vector3 operator+=(const Vector3& v);
	Vector3 operator-=(const Vector3& v);
	Vector3 operator*=(float s);
	Vector3 operator/=(float s);
	
	Vector3 operator+() const;
	Vector3 operator-() const;

	//内積
	float Dot(const Vector3& other) const;
	//外積
	Vector3 Cross(const Vector3& other) const;
	//ベクトルの長さ
	float Length() const;
	//正規化
	Vector3 Normalize() const;
};

Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(float s, const Vector3& v);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator*(const Vector3& v1, const Vector3& v2);
Vector3 operator/(float s, const Vector3& v);
Vector3 operator/(const Vector3& v, float s);

bool operator==(const Vector3& lhs, const Vector3& rhs);
bool operator!=(const Vector3& lhs, const Vector3& rhs);
bool operator<(const Vector3& lhs, const Vector3& rhs);
bool operator<=(const Vector3& lhs, const Vector3& rhs);
bool operator>(const Vector3& lhs, const Vector3& rhs);
bool operator>=(const Vector3& lhs, const Vector3& rhs);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)