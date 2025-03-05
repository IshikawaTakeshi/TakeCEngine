#pragma once
#include "Collision/Collider.h"

class BoxCollider;
class SphereCollider : public Collider {
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) override;

	void Update() override;

	//衝突判定
	bool CheckCollision(Collider* other) override;

	//当たり判定範囲の描画
	void DrawCollider() override;

	Vector3 GetWorldPos() override;

	//中心座標の取得
	Vector3 GetCenterPos() const { return centerPos_; }

	//半径の取得
	float GetRadius() const { return radius_; }

	//OBBとの衝突判定
	bool CheckCollisionOBB(BoxCollider* otherBox);

	//Sphereとの衝突判定
	bool CheckCollisionSphere(SphereCollider* sphere);

private:

	Vector3 centerPos_;

	float radius_;

};

