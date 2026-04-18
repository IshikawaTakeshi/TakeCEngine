#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

// クォータニオン構造体
struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

Quaternion operator*(const Quaternion& q, float s);
Quaternion operator*(float s, const Quaternion& q);
Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator/(const Quaternion& q, float s);
Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator-(const Quaternion& lhs);

// JSON形式に変換
void to_json(nlohmann::json& j, const Quaternion& q);

// JSON形式からQuaternionに変換
void from_json(const nlohmann::json& j, Quaternion& q);

//=============================================================================
// QuaternionMath namespace
//=============================================================================
namespace QuaternionMath {
	//クォータニオン掛け算
	Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
	//スカラー倍
	Quaternion Multiply(const Quaternion& q, float s);
	//内積
	float Dot(const Quaternion& lhs, const Quaternion& rhs);
	//単位クォータニオン
	Quaternion IdentityQuaternion();
	//共役
	Quaternion Conjugate(const Quaternion& q);
	//ノルム
	float Norm(const Quaternion& q);
	//正規化
	Quaternion Normalize(const Quaternion& q);
	//逆クォータニオン
	Quaternion Inverse(const Quaternion& q);
	//任意軸回転クォータニオン
	Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);
	//2つのベクトル間の回転クォータニオン
	Quaternion LookRotation(const Vector3& forward, const Vector3& up);
	Quaternion LookRotation_RowMajor(const Vector3& forwardIn);
	//行列からQuaternionに変換
	Quaternion FromMatrix(const Matrix4x4& m);
	//オイラー角からQuaternionに変換
	Quaternion FromEuler(const Vector3& euler);
	//直交基底からQuaternionに変換
	Quaternion FromBasis(const Vector3& right, const Vector3& up, const Vector3& forward);
	Quaternion FromBasis_RowMajor(const Vector3& right, const Vector3& up, const Vector3& forward);
	//Quaternionの回転結果をベクトルで返す
	Vector3 RotateVector(const Vector3& vector,const Quaternion& quaternion);

	//QuaternionからVector3のオイラー角に変換
	Vector3 ToEuler(const Quaternion& q);
	
}