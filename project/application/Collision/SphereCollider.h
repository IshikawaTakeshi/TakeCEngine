#pragma once
#include "Collision/Collider.h"

class BoxCollider;
class SphereCollider : public Collider {
public:
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	//void OnCollision([[maybe_unused]] Collider* other) override;
	bool CheckCollision(Collider* other) override;

	Vector3 GetCenterPos() const { return centerPos_; }

	float GetRadius() const { return radius_; }


	bool CheckCollisionOBB(BoxCollider* otherBox);

	bool CheckCollisionSphere(SphereCollider* sphere);

private:

	Vector3 centerPos_;

	float radius_;

};

