#define NOMINMAX
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Model.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "TakeCFrameWork.h"
#include <algorithm>

//=============================================================================
// 初期化
//=============================================================================

void SphereCollider::Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) {

	dxCommon_ = dxCommon;
	colliderFilePath_ = "sphere.obj";

	transform_.translate = collisionObject->GetTranslate();
	radius_ = collisionObject->GetScale().Length() / 2.0f;

	//モデルの生成
	model_ = ModelManager::GetInstance()->FindModel(colliderFilePath_);

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = CameraManager::GetInstance()->GetActiveCamera();
}

//=============================================================================
// 更新処理
//=============================================================================

void SphereCollider::Update(EulerTransform transform) {

	transform_ = transform;

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}
	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = model_->GetModelData().rootNode.localMatrix * WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

//=============================================================================
// 衝突判定
//=============================================================================

bool SphereCollider::CheckCollision(Collider* other) {

	// OBBとの衝突処理
	if (BoxCollider* box = dynamic_cast<BoxCollider*>(other)) {
		return CheckCollisionOBB(box);
	}
	// Sphereとの衝突処理
	if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(other)) {
		return CheckCollisionSphere(sphere);
	}

	return false;
}

//=============================================================================
// 当たり判定範囲の描画
//=============================================================================

void SphereCollider::DrawCollider() {


	TakeCFrameWork::GetWireFrame()->DrawSphere(transform_.translate, radius_, { 1.0f,0.0f,0.0f,1.0f });
}

Vector3 SphereCollider::GetWorldPos() {
	
	return transform_.translate;
}

bool SphereCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	Vector3 closestPoint = otherBox->GetOBB().center;

	for (int i = 0; i < 3; i++) {
		float distance = Vector3(transform_.translate - otherBox->GetOBB().center).Dot(otherBox->GetOBB().axis[i]);
		float clampedDistance = std::min(distance, otherBox->GetOBB().halfSize.Dot(otherBox->GetOBB().axis[i]));
		clampedDistance = std::max(-otherBox->GetOBB().halfSize.Dot(otherBox->GetOBB().axis[i]), clampedDistance);

		closestPoint = closestPoint + otherBox->GetOBB().axis[i] * distance;
	}

	Vector3 diff = transform_.translate - closestPoint;
	return diff.Dot(diff) <= (radius_ * radius_);
}

bool SphereCollider::CheckCollisionSphere(SphereCollider* sphere) {
	Vector3 diff = transform_.translate - sphere->transform_.translate;
	float distanceSquared = diff.Dot(diff);
	float radiusSum = radius_ + sphere->radius_;
	return distanceSquared <= (radiusSum * radiusSum);
}