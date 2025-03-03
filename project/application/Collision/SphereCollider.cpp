#define NOMINMAX
#include "SphereCollider.h"
#include "BoxCollider.h"

#include <algorithm>

void SphereCollider::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	collisionObject_ = std::make_unique<Object3d>();
	collisionObject_->Initialize(object3dCommon, filePath);

	centerPos_ = collisionObject_->GetPosition();
	radius_ = collisionObject_->GetScale().Length() / 2.0f;
}

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