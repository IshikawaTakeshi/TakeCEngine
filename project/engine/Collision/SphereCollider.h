#pragma once
#include "Collision/Collider.h"
#include "engine/math/OBB.h"

class BoxCollider;
class SphereCollider : public Collider {
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) override;

	void Update(Object3d* collisionObject) override;
	void UpdateImGui([[maybe_unused]]const std::string& name) override;
	//衝突判定
	bool CheckCollision(Collider* other) override;

	//当たり判定範囲の描画
	void DrawCollider() override;
	// レイとの衝突判定
	bool Intersects(const Ray& ray, RayCastHit& outHit) override;
	//衝突面のタイプを判定
	SurfaceType CheckSurfaceType() const;

	Vector3 GetWorldPos() override;

	//半径の取得
	float GetRadius() const { return radius_; }

	void SetRadius(const float& radius) override { radius_ = radius; }

	//OBBとの衝突判定
	bool CheckCollisionOBB(BoxCollider* otherBox);

	//Sphereとの衝突判定
	bool CheckCollisionSphere(SphereCollider* sphere);

private:

	Vector3 SphereCenterToOBBLocal(const OBB& obb, const Vector3& sphereCenter);

	float radius_;
};

