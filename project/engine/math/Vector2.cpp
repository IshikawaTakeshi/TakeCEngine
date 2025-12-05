#include "Vector2.h"
#include <cmath>
#include <algorithm>
#include <tuple>

//=============================================================================
// operator overloads
//=============================================================================

Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}


Vector2 operator*(float scalar,const Vector2& v) {
	return { v.x * scalar, v.y * scalar };
}

Vector2 operator*(const Vector2& v, float scalar) {
	return { v.x * scalar, v.y * scalar };
}

Vector2 operator/(const Vector2& v,float scalar) {
	if (scalar == 0.0f) {
		return { 0.0f, 0.0f }; // Avoid division by zero
	}
	return { v.x / scalar, v.y / scalar };
}

bool operator==(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x == rhs.x && lhs.y == rhs.y);
}

bool operator<=(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x <= rhs.x && lhs.y <= rhs.y);
}

bool operator>=(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x >= rhs.x && lhs.y >= rhs.y);
}

bool operator!=(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x != rhs.x || lhs.y != rhs.y);
}

bool operator<(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x < rhs.x && lhs.y < rhs.y);
}

bool operator>(const Vector2& lhs, const Vector2& rhs) {
	return (lhs.x > rhs.x && lhs.y > rhs.y);
}

Vector2 Vector2::operator+() const{
	return {x, y};
}

Vector2 Vector2::operator-() const{
	return { -x, -y };
}

Vector2 Vector2::operator+=(const Vector2& rhs) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vector2 Vector2::operator-=(const Vector2& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vector2 Vector2::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	return *this;
}

Vector2 Vector2::operator/=(float scalar) {
	if (scalar == 0.0f) {
		return { 0.0f, 0.0f }; // Avoid division by zero
	}
	x /= scalar;
	y /= scalar;
	return *this;
}


//=============================================================================
// member functions
//=============================================================================

float Vector2::Length() const {
	return sqrt(x * x + y * y);
}

Vector2 Vector2::Normalize() const {
	float len = Length();
	if (len == 0.0f) {
		return { 0.0f, 0.0f }; // Avoid division by zero
	}
	return { x / len, y / len };
}
float Vector2::Dot(const Vector2& other) const {
	return x * other.x + y * other.y;
}
float Vector2::Cross(const Vector2& other) const {
	return x * other.y - y * other.x;
}