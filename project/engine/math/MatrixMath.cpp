#include "MatrixMath.h"
#include "Vector3Math.h"
#include <assert.h>
#include <cmath>
#include <xmmintrin.h>

//============================================================================
//行列の加法
//============================================================================
Matrix4x4 MatrixMath::Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = m1 + m2;
	return result;
}

//============================================================================
//行列の減法
//============================================================================
Matrix4x4 MatrixMath::Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = m1 - m2;
	return result;
}

//============================================================================
//行列の積
//============================================================================
Matrix4x4 MatrixMath::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result; for (int row = 0; row < 4; row++) { // m1 の1行を4要素同時ロード
		__m128 r = _mm_set_ps(m1.m[row][3], m1.m[row][2], m1.m[row][1], m1.m[row][0]);
		for (int col = 0; col < 4; col++) { 
			// m2 の1列を4要素同時ロード
			__m128 c = _mm_set_ps(m2.m[3][col], m2.m[2][col], m2.m[1][col], m2.m[0][col]);
			// 4要素の積を同時計算し、水平加算
			__m128 mul = _mm_mul_ps(r, c); 
			// 水平加算: (a+b) + (c+d)
			__m128 hadd = _mm_add_ps(mul, _mm_movehl_ps(mul, mul)); 
			hadd = _mm_add_ss(hadd, _mm_shuffle_ps(hadd, hadd, 1));
			result.m[row][col] = _mm_cvtss_f32(hadd);
		}
	}
	return result;
}

//============================================================================
//逆行列
//============================================================================
Matrix4x4 MatrixMath::Inverse(const Matrix4x4& m) {
	float determinant =
		(m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3])
		+ (m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1])
		+ (m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2])
		- (m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1])
		- (m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3])
		- (m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2])
		- (m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3])
		- (m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1])
		- (m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2])
		+ (m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1])
		+ (m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3])
		+ (m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2])
		+ (m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3])
		+ (m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1])
		+ (m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2])
		- (m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1])
		- (m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3])
		- (m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2])
		- (m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0])
		- (m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0])
		- (m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0])
		+ (m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0])
		+ (m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0])
		+ (m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0]);

	Matrix4x4 result;
	result.m[0][0] = 1.0f / determinant *
		(m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2]
			- m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][1] = 1.0f / determinant *
		(-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2]
			+ m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][2] = 1.0f / determinant *
		(m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2]
			- m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);

	result.m[0][3] = 1.0f / determinant *
		(-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2]
			+ m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);

	result.m[1][0] = 1.0f / determinant *
		(-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2]
			+ m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][1] = 1.0f / determinant *
		(m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2]
			- m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][2] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2]
			+ m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);

	result.m[1][3] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2]
			- m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

	result.m[2][0] = 1.0f / determinant *
		(m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1]
			- m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][1] = 1.0f / determinant *
		(-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1]
			+ m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][2] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1]
			- m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);

	result.m[2][3] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1]
			+ m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);

	result.m[3][0] = 1.0f / determinant *
		(-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1]
			+ m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][1] = 1.0f / determinant *
		(m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1]
			- m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][2] = 1.0f / determinant *
		(-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1]
			+ m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);

	result.m[3][3] = 1.0f / determinant *
		(m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1]
			- m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

	return result;
}

//============================================================================
//転置行列
//============================================================================
Matrix4x4 MatrixMath::Transpose(const Matrix4x4& m) {
	Matrix4x4 result;

	__m128 row0 = _mm_loadu_ps(m.m[0]);
	__m128 row1 = _mm_loadu_ps(m.m[1]);
	__m128 row2 = _mm_loadu_ps(m.m[2]);
	__m128 row3 = _mm_loadu_ps(m.m[3]);

	_MM_TRANSPOSE4_PS(row0, row1, row2, row3);

	_mm_storeu_ps(result.m[0], row0);
	_mm_storeu_ps(result.m[1], row1);
	_mm_storeu_ps(result.m[2], row2);
	_mm_storeu_ps(result.m[3], row3);

	return result;
}

//============================================================================
//単位行列の作成
//============================================================================
Matrix4x4 MatrixMath::MakeIdentity4x4() {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//平行移動行列
//============================================================================
Matrix4x4 MatrixMath::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

//============================================================================
//拡大縮小行列
//============================================================================
Matrix4x4 MatrixMath::MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = {};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//X軸回転行列
//============================================================================
Matrix4x4 MatrixMath::MakeRotateXMatrix(float radian) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = std::sin(-radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//Y軸回転行列
//============================================================================
Matrix4x4 MatrixMath::MakeRotateYMatrix(float radian) {
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radian);
	result.m[0][2] = std::sin(-radian);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//Z軸回転行列
//============================================================================
Matrix4x4 MatrixMath::MakeRotateZMatrix(float radian) {
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = std::sin(-radian);
	result.m[1][1] = std::cos(radian);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//回転行列（オイラー角）
//============================================================================
Matrix4x4 MatrixMath::MakeRotateMatrix(const Vector3& rotate) {
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
	return rotateMatrix;
}

//============================================================================
//任意軸回転行列
//============================================================================
Matrix4x4 MatrixMath::MakeRotateAxisAngle(const Vector3& axis, float angle) {
	float cosA = std::cos(angle);
	float sinA = std::sin(angle);
	float oneMinusCosA = 1.0f - cosA;

	Matrix4x4 result;
	result.m[0][0] = axis.x * axis.x * oneMinusCosA + cosA;
	result.m[0][1] = axis.x * axis.y * oneMinusCosA + axis.z * sinA;
	result.m[0][2] = axis.x * axis.z * oneMinusCosA - axis.y * sinA;
	result.m[0][3] = 0.0f;

	result.m[1][0] = axis.y * axis.x * oneMinusCosA - axis.z * sinA;
	result.m[1][1] = axis.y * axis.y * oneMinusCosA + cosA;
	result.m[1][2] = axis.y * axis.z * oneMinusCosA + axis.x * sinA;
	result.m[1][3] = 0.0f;

	result.m[2][0] = axis.z * axis.x * oneMinusCosA + axis.y * sinA;
	result.m[2][1] = axis.z * axis.y * oneMinusCosA - axis.x * sinA;
	result.m[2][2] = axis.z * axis.z * oneMinusCosA + cosA;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

//============================================================================
//回転行列（クォータニオン）
//============================================================================
Matrix4x4 MatrixMath::MakeRotateMatrix(const Quaternion& quaternion) {
	Matrix4x4 result;
	result.m[0][0] = 1.0f - 2.0f * quaternion.y * quaternion.y - 2.0f * quaternion.z * quaternion.z;
	result.m[0][1] = 2.0f * quaternion.x * quaternion.y + 2.0f * quaternion.w * quaternion.z;
	result.m[0][2] = 2.0f * quaternion.x * quaternion.z - 2.0f * quaternion.w * quaternion.y;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 2.0f * quaternion.x * quaternion.y - 2.0f * quaternion.w * quaternion.z;
	result.m[1][1] = 1.0f - 2.0f * quaternion.x * quaternion.x - 2.0f * quaternion.z * quaternion.z;
	result.m[1][2] = 2.0f * quaternion.y * quaternion.z + 2.0f * quaternion.w * quaternion.x;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 2.0f * quaternion.x * quaternion.z + 2.0f * quaternion.w * quaternion.y;
	result.m[2][1] = 2.0f * quaternion.y * quaternion.z - 2.0f * quaternion.w * quaternion.x;
	result.m[2][2] = 1.0f - 2.0f * quaternion.x * quaternion.x - 2.0f * quaternion.y * quaternion.y;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

//============================================================================
//3次元アフィン変換行列
//============================================================================
Matrix4x4 MatrixMath::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 result = {};
	result.m[0][0] = scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = scale.x * rotateMatrix.m[0][2];
	result.m[1][0] = scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = scale.y * rotateMatrix.m[1][2];
	result.m[2][0] = scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = scale.z * rotateMatrix.m[2][2];
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MatrixMath::MakeAffineMatrix(const EulerTransform& transform) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(transform.rotate);
	Matrix4x4 result = {};
	result.m[0][0] = transform.scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = transform.scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = transform.scale.x * rotateMatrix.m[0][2];
	result.m[1][0] = transform.scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = transform.scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = transform.scale.y * rotateMatrix.m[1][2];
	result.m[2][0] = transform.scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = transform.scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = transform.scale.z * rotateMatrix.m[2][2];
	result.m[3][0] = transform.translate.x;
	result.m[3][1] = transform.translate.y;
	result.m[3][2] = transform.translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//3次元アフィン変換行列（クォータニオン）
//============================================================================
Matrix4x4 MatrixMath::MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(rotate);
	Matrix4x4 result = {};
	result.m[0][0] = scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = scale.x * rotateMatrix.m[0][2];
	result.m[1][0] = scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = scale.y * rotateMatrix.m[1][2];
	result.m[2][0] = scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = scale.z * rotateMatrix.m[2][2];
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MatrixMath::MakeAffineMatrix(const QuaternionTransform& transform) {
	Matrix4x4 rotateMatrix = MakeRotateMatrix(transform.rotate);
	Matrix4x4 result = {};
	result.m[0][0] = transform.scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = transform.scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = transform.scale.x * rotateMatrix.m[0][2];
	result.m[1][0] = transform.scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = transform.scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = transform.scale.y * rotateMatrix.m[1][2];
	result.m[2][0] = transform.scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = transform.scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = transform.scale.z * rotateMatrix.m[2][2];
	result.m[3][0] = transform.translate.x;
	result.m[3][1] = transform.translate.y;
	result.m[3][2] = transform.translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//透視投影行列
//============================================================================
Matrix4x4 MatrixMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float f = 1.0f / std::tan(fovY / 2.0f);
	result.m[0][0] = f / aspectRatio;
	result.m[1][1] = f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -(farClip * nearClip) / (farClip - nearClip);
	return result;
}

//============================================================================
//正射影行列
//============================================================================
Matrix4x4 MatrixMath::MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};

	float width = right - left;
	float height = top - bottom;
	float depth = farClip - nearClip;

	result.m[0][0] = 2.0f / width;
	result.m[1][1] = 2.0f / height;
	result.m[2][2] = 1.0f / depth;
	result.m[3][0] = -(right + left) / width;
	result.m[3][1] = -(top + bottom) / height;
	result.m[3][2] = -nearClip / depth;
	result.m[3][3] = 1.0f;

	return result;
}

//============================================================================
//ビューポート変換行列
//============================================================================
Matrix4x4 MatrixMath::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f; // Y軸を反転
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

//============================================================================
//Matrix4x4からVector3に座標変換
//============================================================================
Vector3 MatrixMath::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

//============================================================================
//Matrix4x4からVector3に法線変換
//============================================================================
Vector3 MatrixMath::TransformNormal(const Vector3& v, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = v.x * matrix.m[0][0] + v.y * matrix.m[1][0] + v.z * matrix.m[2][0];
	result.y = v.x * matrix.m[0][1] + v.y * matrix.m[1][1] + v.z * matrix.m[2][1];
	result.z = v.x * matrix.m[0][2] + v.y * matrix.m[1][2] + v.z * matrix.m[2][2];

	return result;
}

//============================================================================
//逆転置行列
//============================================================================
Matrix4x4 MatrixMath::InverseTranspose(const Matrix4x4& m) {

	Matrix4x4 result = Inverse(m);
	result = Transpose(result);
	return result;
}

//============================================================================
//fromベクトルからtoベクトルへの回転行列を作成
//============================================================================
Matrix4x4 MatrixMath::DirectionToDirection(const Vector3& from, const Vector3& to) {

	Vector3 crossProduct = Vector3Math::Cross(from, to);
	float cosA = Vector3Math::Dot(from, to);
	float sinA = Vector3Math::Length(crossProduct);
	float oneMinusCosA = 1.0f - cosA;

	Vector3 axis;
	if (sinA < 1e-6) {
		// from と to が反対方向を向いている場合
		if (fabs(from.x) > fabs(from.y)) {
			axis = Vector3Math::Normalize(Vector3{ from.y,-from.x,0.0f });
		} else {
			axis = Vector3Math::Normalize(Vector3{ from.z, 0.0f, -from.x });
		}
	} else {
		axis = Vector3Math::Normalize(crossProduct);
	}

	Matrix4x4 result;
	result.m[0][0] = axis.x * axis.x * oneMinusCosA + cosA;
	result.m[0][1] = axis.x * axis.y * oneMinusCosA + axis.z * sinA;
	result.m[0][2] = axis.x * axis.z * oneMinusCosA - axis.y * sinA;
	result.m[0][3] = 0.0f;

	result.m[1][0] = axis.y * axis.x * oneMinusCosA - axis.z * sinA;
	result.m[1][1] = axis.y * axis.y * oneMinusCosA + cosA;
	result.m[1][2] = axis.y * axis.z * oneMinusCosA + axis.x * sinA;
	result.m[1][3] = 0.0f;

	result.m[2][0] = axis.z * axis.x * oneMinusCosA + axis.y * sinA;
	result.m[2][1] = axis.z * axis.y * oneMinusCosA - axis.x * sinA;
	result.m[2][2] = axis.z * axis.z * oneMinusCosA + cosA;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MatrixMath::LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
	
	// 1. Z 軸（視線方向）を計算
	Vector3 zAxis = target - eye;

	// eye と target が同じ位置の場合のフォールバック
	float zLength = Vector3Math:: Length(zAxis);
	if (zLength < 1e-6f) {
		zAxis = Vector3(0.0f, 0.0f, 1.0f);  // デフォルトの前方向
	} else {
		zAxis = zAxis / zLength;  // 正規化
	}

	// 2. X 軸（右方向）を計算
	Vector3 xAxis = Vector3Math::Cross(up, zAxis);

	// up と zAxis が平行な場合のフォールバック
	float xLength = Vector3Math::Length(xAxis);
	if (xLength < 1e-6f) {
		// 別の up ベクトルを試す
		Vector3 altUp = (std::abs(zAxis.y) < 0.99f) 
			? Vector3(0.0f, 1.0f, 0.0f) 
			: Vector3(1.0f, 0.0f, 0.0f);
		xAxis = Vector3Math:: Cross(altUp, zAxis);
		xLength = Vector3Math::Length(xAxis);
	}
	xAxis = xAxis / xLength;  // 正規化

	// 3. Y 軸（上方向）を計算（正規化済みの x, z から計算するので正規化不要）
	Vector3 yAxis = Vector3Math::Cross(zAxis, xAxis);

	// 4. ビュー行列を構築
	Matrix4x4 result = {};
	result.m[0][0] = xAxis.x;
	result.m[0][1] = yAxis.x;
	result.m[0][2] = zAxis.x;
	result.m[0][3] = 0.0f;
	result.m[1][0] = xAxis.y;
	result.m[1][1] = yAxis.y;
	result.m[1][2] = zAxis.y;
	result.m[1][3] = 0.0f;
	result.m[2][0] = xAxis.z;
	result.m[2][1] = yAxis.z;
	result.m[2][2] = zAxis.z;
	result.m[2][3] = 0.0f;
	result.m[3][0] = -Vector3Math::Dot(xAxis, eye);
	result.m[3][1] = -Vector3Math::Dot(yAxis, eye);
	result.m[3][2] = -Vector3Math::Dot(zAxis, eye);
	result.m[3][3] = 1.0f;
	return result;
}
