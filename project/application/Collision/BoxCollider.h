#pragma once
#include "Collision/Collider.h"
#include "OBB.h"

class SphereCollider;
class BoxCollider : public Collider {

public:

	BoxCollider() = default;
	~BoxCollider();

	//初期化
	void Initialize(DirectXCommon* dxCommon ,Object3d* collisionObject) override;

	void Update() override;

	//衝突判定
	bool CheckCollision(Collider* other) override;

	//当たり判定範囲の描画
	void DrawCollider() override;

	Vector3 GetWorldPos() override;

	OBB GetOBB() { return obb_; }

	Matrix4x4 GetRotateMatrix() { return rotateMatrix_; }

	//OBBとの衝突判定
	bool CheckCollisionOBB(BoxCollider* otherBox);

private: // privateメンバ変数
	//衝突OBB
	OBB obb_;
	Matrix4x4 rotateMatrix_;
};

