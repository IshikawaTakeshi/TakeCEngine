#pragma once
#include <json.hpp>
#include <compare>

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;

	Vector2 operator+() const;
	Vector2 operator-() const;
	
	Vector2 operator+=(const Vector2& rhs);
	Vector2 operator-=(const Vector2& rhs);
	Vector2 operator*=(float scalar);
	Vector2 operator/=(float scalar);

	// 長さを求める
	float Length() const;
	// 正規化
	Vector2 Normalize() const;
	// 内積
	float Dot(const Vector2& other) const;
	// 外積
	float Cross(const Vector2& other) const;
};

Vector2 operator+(const Vector2& lhs, const Vector2& rhs);
Vector2 operator-(const Vector2& lhs, const Vector2& rhs);
Vector2 operator*(float scalar,const Vector2& v);
Vector2 operator*(const Vector2& v,float scalar);
Vector2 operator/(const Vector2& v,float scalar);

bool operator==(const Vector2& lhs, const Vector2& rhs);
bool operator<=(const Vector2& lhs, const Vector2& rhs);
bool operator>=(const Vector2& lhs, const Vector2& rhs);
bool operator!=(const Vector2& lhs, const Vector2& rhs);
bool operator<(const Vector2& lhs, const Vector2& rhs);
bool operator>(const Vector2& lhs, const Vector2& rhs);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y)