#include "Matrix4x4.h"
#include "MatrixMath.h"

Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Add(m1, m2);
}

Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Subtract(m1, m2);
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	return MatrixMath::Multiply(m1, m2);
}

Matrix4x4 operator*(const Matrix4x4& m, float scalar) {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.m[row][column] = m.m[row][column] * scalar;
		}
	}
	return result;
}

Matrix4x4::Matrix4x4(const Matrix4x4& other) {

	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			this->m[row][column] = other.m[row][column];
		}
	}
}

Matrix4x4 Matrix4x4::operator+() const {
	return *this;
}

Matrix4x4 Matrix4x4::operator-() const {
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			result.m[row][column] = -this->m[row][column];
		}
	}
	return result;
}

Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& matrix) {
	
	this->m[0][0] += matrix.m[0][0];
	this->m[0][1] += matrix.m[0][1];
	this->m[0][2] += matrix.m[0][2];
	this->m[0][3] += matrix.m[0][3];
	this->m[1][0] += matrix.m[1][0];
	this->m[1][1] += matrix.m[1][1];
	this->m[1][2] += matrix.m[1][2];
	this->m[1][3] += matrix.m[1][3];
	this->m[2][0] += matrix.m[2][0];
	this->m[2][1] += matrix.m[2][1];
	this->m[2][2] += matrix.m[2][2];
	this->m[2][3] += matrix.m[2][3];
	this->m[3][0] += matrix.m[3][0];
	this->m[3][1] += matrix.m[3][1];
	this->m[3][2] += matrix.m[3][2];
	this->m[3][3] += matrix.m[3][3];
	return *this;
}

Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& matrix) {
	this->m[0][0] -= matrix.m[0][0];
	this->m[0][1] -= matrix.m[0][1];
	this->m[0][2] -= matrix.m[0][2];
	this->m[0][3] -= matrix.m[0][3];
	this->m[1][0] -= matrix.m[1][0];
	this->m[1][1] -= matrix.m[1][1];
	this->m[1][2] -= matrix.m[1][2];
	this->m[1][3] -= matrix.m[1][3];
	this->m[2][0] -= matrix.m[2][0];
	this->m[2][1] -= matrix.m[2][1];
	this->m[2][2] -= matrix.m[2][2];
	this->m[2][3] -= matrix.m[2][3];
	this->m[3][0] -= matrix.m[3][0];
	this->m[3][1] -= matrix.m[3][1];
	this->m[3][2] -= matrix.m[3][2];
	this->m[3][3] -= matrix.m[3][3];
	return *this;
}

Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& matrix) {
	
	Matrix4x4 result;
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			m[row][column] = 0;
			for (int k = 0; k < 4; k++) {
				result.m[row][column] += m[row][k] * matrix.m[k][column];
			}
		}
	}
	*this = result;

	return *this;
}
