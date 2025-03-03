#define NOMINMAX
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Model.h"
#include "ModelManager.h"

#include <algorithm>

//=============================================================================
// 初期化
//=============================================================================

void SphereCollider::Initialize(Object3d* collisionObject, const std::string& filePath) {

	centerPos_ = collisionObject->GetPosition();
	radius_ = collisionObject->GetScale().Length() / 2.0f;

	collisionModel_ = ModelManager::GetInstance()->FindModel(filePath);
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

	collisionModel_->Draw();
}

bool SphereCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	Vector3 closestPoint = otherBox->GetOBB().center;

	for (int i = 0; i < 3; i++) {
		float distance = Vector3(centerPos_ - otherBox->GetOBB().center).Dot(otherBox->GetOBB().axis[i]);
		float clampedDistance = std::min(distance, otherBox->GetOBB().halfSize.Dot(otherBox->GetOBB().axis[i]));
		clampedDistance = std::max(-otherBox->GetOBB().halfSize.Dot(otherBox->GetOBB().axis[i]), clampedDistance);

		closestPoint = closestPoint + otherBox->GetOBB().axis[i] * distance;
	}

	Vector3 diff = centerPos_ - closestPoint;
	return diff.Dot(diff) <= (radius_ * radius_);
}

bool SphereCollider::CheckCollisionSphere(SphereCollider* sphere) {
	Vector3 diff = centerPos_ - sphere->centerPos_;
	float distanceSquared = diff.Dot(diff);
	float radiusSum = radius_ + sphere->radius_;
	return distanceSquared <= (radiusSum * radiusSum);
}