#pragma once

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;

	Vector2 operator+(const Vector2& rhs) const;
	Vector2 operator+();
	Vector2 operator-(const Vector2& rhs) const;
	Vector2 operator-();
	Vector2 operator*(float scalar) const;
	Vector2 operator/(float scalar) const;
	bool operator==(const Vector2& rhs) const;
	bool operator<=(const Vector2& rhs) const;
	bool operator>=(const Vector2& rhs) const;
	bool operator!=(const Vector2& rhs) const;
	bool operator<(const Vector2& rhs) const;
	bool operator>(const Vector2& rhs) const;

	Vector2 operator+=(const Vector2& rhs);
	Vector2 operator-=(const Vector2& rhs);
	Vector2 operator*=(float scalar);
	Vector2 operator/=(float scalar);
	float Length() const;
	Vector2 Normalize() const;
	float Dot(const Vector2& other) const;
	float Cross(const Vector2& other) const;
};