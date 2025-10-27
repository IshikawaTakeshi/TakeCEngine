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

	// 長さを求める
	float Length() const;
	// 正規化
	Vector2 Normalize() const;
	// 内積
	float Dot(const Vector2& other) const;
	// 外積
	float Cross(const Vector2& other) const;
};