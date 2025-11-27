#include "Vector2.h"
#include <cmath>
#include <algorithm>

//=============================================================================
// operator overloads
//=============================================================================

Vector2 Vector2::operator+(){
	return {x, y};
}

Vector2 Vector2::operator+(const Vector2& rhs) const {
	return { x + rhs.x, y + rhs.y };
}

Vector2 Vector2::operator-(const Vector2& rhs) const {
	return { x - rhs.x, y - rhs.y };
}

Vector2 Vector2::operator-() {
	return { -x, -y };
}

Vector2 Vector2::operator*(float scalar) const {
	return { x * scalar, y * scalar };
}

Vector2 Vector2::operator/(float scalar) const {
	if (scalar == 0.0f) {
		return { 0.0f, 0.0f }; // Avoid division by zero
	}
	return { x / scalar, y / scalar };
}

bool Vector2::operator==(const Vector2& rhs) const {
	return (x == rhs.x && y == rhs.y);
}

bool Vector2::operator<=(const Vector2& rhs) const {
	return (x <= rhs.x && y <= rhs.y);
}

bool Vector2::operator>=(const Vector2& rhs) const {
	return (x >= rhs.x && y >= rhs.y);
}

bool Vector2::operator!=(const Vector2& rhs) const {
	return (x != rhs.x || y != rhs.y);
}

bool Vector2::operator<(const Vector2& rhs) const {
	return (x < rhs.x && y < rhs.y);
}

bool Vector2::operator>(const Vector2& rhs) const {
	return (x > rhs.x && y > rhs.y);
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

//=============================================================================
// JSON形式に変換
//=============================================================================
void to_json(nlohmann::json& j, const Vector2& v) {

	j = nlohmann::json{ 
		{"x", v.x},
		{"y", v.y}
	};
}

//=============================================================================
// JSON形式からVector2に変換
//=============================================================================
void from_json(const nlohmann::json& j, Vector2& v) {

	j.at("x").get_to(v.x);
	j.at("y").get_to(v.y);
}