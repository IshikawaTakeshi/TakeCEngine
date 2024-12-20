#include "Camera.h"
#include "MatrixMath.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"

Camera::~Camera() {}

void Camera::Initialize(ID3D12Device* device) {
	
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	offset_ = { 0.0f, 0.0f, -25.0f };
	fovX_ = 0.45f;
	aspectRatio_ = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	nearClip_ = 0.1f;
	farClip_ = 100.0f;
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(fovX_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);

	rotationMatrix_ = MatrixMath::MakeIdentity4x4();
	//rotationMatrixDelta_ = MatrixMath::MakeIdentity4x4();

	cameraResource_ = DirectXCommon::CreateBufferResource(device, sizeof(CameraForGPU));
	cameraForGPU_ = nullptr;
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
	cameraForGPU_->worldPosition = transform_.translate;
}

void Camera::Update() {

	//追加回転分の回転行列の計算
	Matrix4x4 rotationMatrixDelta_ = MatrixMath::MakeIdentity4x4();

	if (Input::GetInstance()->IsPressMouse(1)) {
		rotationMatrixDelta_ = rotationMatrixDelta_ * MatrixMath::MakeRotateXMatrix((float)Input::GetInstance()->GetMouseMove().lY * 0.001f);
		rotationMatrixDelta_ = rotationMatrixDelta_ * MatrixMath::MakeRotateYMatrix((float)Input::GetInstance()->GetMouseMove().lX * 0.001f);
	}

	//累積回転行列の計算
	rotationMatrix_ = rotationMatrix_ * rotationMatrixDelta_;

	if (Input::GetInstance()->IsPressMouse(2)) {
		offsetDelta_.x += (float)Input::GetInstance()->GetMouseMove().lX * 0.01f;
		offsetDelta_.y -= (float)Input::GetInstance()->GetMouseMove().lY * 0.01f;
	}
	//オフセットを考慮したワールド行列の計算
	offsetDelta_.z += (float)Input::GetInstance()->GetWheel() * 0.01f;
	offset_ = offsetDelta_;
	
	offset_ = MatrixMath::TransformNormal(offset_, rotationMatrix_);
	transform_.translate = offset_;
	
	Matrix4x4 translationMatrix = MatrixMath::MakeTranslateMatrix(transform_.translate);
	Matrix4x4 scaleMatrix = MatrixMath::MakeScaleMatrix(transform_.scale);



	worldMatrix_ = scaleMatrix * rotationMatrix_ * translationMatrix;
	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);

	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		fovX_,
		aspectRatio_,
		nearClip_,
		farClip_
	);

	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);

	cameraForGPU_->worldPosition = transform_.translate;
}

#ifdef _DEBUG
void Camera::UpdateImGui() {
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::DragFloat("FovX", &fovX_, 0.01f);
}

#endif // DEBUG