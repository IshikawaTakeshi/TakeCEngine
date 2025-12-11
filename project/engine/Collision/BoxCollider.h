#pragma once
#include "Collision/Collider.h"
#include "OBB.h"

//前方宣言
class SphereCollider;

//=============================================================================
// BoxCollider class
//=============================================================================
class BoxCollider : public Collider {
public:

	//=========================================================================
	// functions
	//=========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="collisionObject"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon ,Object3d* collisionObject) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="collisionObject"></param>
	void Update(Object3d* collisionObject) override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name="name"></param>
	void UpdateImGui([[maybe_unused]]const std::string& name) override;

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	bool CheckCollision(Collider* other) override;
	
	/// <summary>
	/// レイとの当たり判定
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="outHit"></param>
	/// <returns></returns>
	bool Intersects(const Ray& ray, RayCastHit& outHit) override;

	/// <summary>
	/// 当たり判定の描画
	/// </summary>
	void DrawCollider() override;

	/// <summary>
	/// 法線ベクトルから面の種類を判定
	/// </summary>
	/// <param name="Axis"></param>
	/// <param name="normal"></param>
	/// <returns></returns>
	SurfaceType CheckSurfaceType(const Vector3* Axis, const Vector3& normal) const;


public:

	//=================================================================================
	// accessor
	//=================================================================================

	//----- getter ---------------

	//ワールド座標の取得
	Vector3 GetWorldPos() override;
	//半径の取得
	void SetHalfSize(const Vector3& halfSize) override;
	//OBBの取得
	const OBB& GetOBB() { return obb_; }
	//回転行列の取得
	Matrix4x4 GetRotateMatrix() { return rotateMatrix_; }
	//最小分離軸の取得
	Vector3 GetMinAxis() const { return minAxis_; }
	//penetration depthの取得
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