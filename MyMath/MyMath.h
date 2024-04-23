#pragma once
#include <vector3.h>

class MyMath {
public:
	//加算
	static Vector3 Add(const Vector3& v1, const Vector3& v2);

	//減算
	static Vector3 Subtract(const Vector3& v1, const Vector3& v2);

	//乗算
	static Vector3 Multiply(float scalar, const Vector3& v);

	//内積
	static float Dot(const Vector3& v1, const Vector3& v2);

	//クロス積
	static Vector3 Cross(const Vector3& v1, const Vector3& v2);

	//長さ(ノルム)
	static float Length(const Vector3& v);

	//正規化
	static Vector3 Normalize(const Vector3& v);

	//数値表示
	static void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label);
};
