#pragma once
#include "Collision/Collider.h"
#include "OBB.h"

class SphereCollider;
class BoxCollider : public Collider {
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon ,Object3d* collisionObject) override;

	void Update(Object3d* collisionObject) override;

	void UpdateImGui([[maybe_unused]]const std::string& name) override;

	//衝突判定
	bool CheckCollision(Collider* other) override;
	// レイとの衝突判定
	bool Intersects(const Ray& ray, RayCastHit& outHit) override;

	//当たり判定範囲の描画
	void DrawCollider() override;

	SurfaceType CheckSurfaceType(const Vector3* Axis, const Vector3& normal) const;


public:

	//ワールド座標の取得
	Vector3 GetWorldPos() override;
	//半径の取得
	void SetHalfSize(const Vector3& halfSize) override;
	//OBBの取得
	const OBB& GetOBB() { return obb_; }
	//回転行列の取得
	Matrix4x4 GetRotateMatrix() { return rotateMatrix_; }

	Vector3 GetMinAxis() const { return minAxis_; }
	float GetMinPenetration() const { return minPenetration_; }

private:

	//OBBとの衝突判定
	bool CheckCollisionOBB(BoxCollider* otherBox);

private: // privateメンバ変数
	//衝突OBB
	OBB obb_;
	Matrix4x4 rotateMatrix_;
	Vector3 minAxis_;    // 最小分離軸（衝突面の法線）
	float minPenetration_; // penetration depth
};