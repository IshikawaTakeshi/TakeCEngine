#pragma once
#include "Collision/Collider.h"
#include "OBB.h"

class SphereCollider;
class BoxCollider : public Collider {

public:

	void Initialize(Object3dCommon* object3dCommon,const std::string& filePath) override;

	//void OnCollision([[maybe_unused]] Collider* other) override;

	bool CheckCollision(Collider* other) override;

	OBB GetOBB() { return obb_; }

	Matrix4x4 GetRotateMatrix() { return rotateMatrix_; }

	bool CheckCollisionOBB(BoxCollider* otherBox);

private: // privateメンバ変数
	//衝突OBB
	OBB obb_;
	Matrix4x4 rotateMatrix_;
};

