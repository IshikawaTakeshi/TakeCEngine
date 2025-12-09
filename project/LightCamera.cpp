#include "LightCamera.h"
#include "engine/math/MatrixMath.h"
#include "engine/base/WinApp.h"

void LightCamera::Initialize(const Vector3& positon) {
	position_ = positon;
	const Matrix4x4& viewMat = MatrixMath::MakeIdentity4x4();

	const Matrix4x4& projectionMat = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, WinApp::kScreenWidth, WinApp::kScreenHeight, nearClip_,farClip_);

	viewProjection_ = projectionMat * viewMat;
}

void LightCamera::Update() {
		const Matrix4x4& viewMat = MatrixMath::MakeIdentity4x4();
	const Matrix4x4& projectionMat = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, WinApp::kScreenWidth, WinApp::kScreenHeight, nearClip_, farClip_);
	viewProjection_ = projectionMat * viewMat;
}