#pragma once
#include "Transform.h"
#include "Matrix4x4.h"

class Camera {
public:
	Camera();
	~Camera();
	void Update();
	void UpdateImGui();

public: //getter
	
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector3& GetRotate() const { return transform_.rotate; }

public: //setter

	void SetTranslate(const Vector3& Translate) { transform_.translate = Translate; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetFovX(const float fovX) { fovX_ = fovX; }
	void SetAspectRatio(const float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(const float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(const float farClip) { farClip_ = farClip; }

private:

	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	
	//水平方向視野角
	float fovX_;
	//aspect比
	float aspectRatio_;
	//nearクリップ距離
	float nearClip_;
	//farクリップ距離
	float farClip_;
};

