#pragma once
#include "Collision/Collider.h"
#include "engine/math/OBB.h"

//前方宣言
class BoxCollider;

//=============================================================================
// SphereCollider class
//=============================================================================
class SphereCollider : public Collider {
public:

	//=========================================================================
	// functions
	//=========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="collisionObject"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon, Object3d* collisionObject) override;

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
	/// コライダーの描画
	/// </summary>
	void DrawCollider() override;
	
	/// <summary>
	/// レイとの衝突判定
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="outHit"></param>
	/// <returns></returns>
	bool Intersects(const Ray& ray, RayCastHit& outHit) override;
	
	/// <summary>
	/// サーフェスタイプの取得
	/// </summary>
	/// <returns></returns>
	SurfaceType CheckSurfaceType() const;

	/// <summary>
	/// OBBとの衝突判定
	/// </summary>
	/// <param name="otherBox"></param>
	/// <returns></returns>
	bool CheckCollisionOBB(BoxCollider* otherBox);

	/// <summary>
	/// 球体同士の衝突判定
	/// </summary>
	/// <param name="sphere"></param>
	/// <returns></returns>
	bool CheckCollisionSphere(SphereCollider* sphere);

private:

	//球の中心座標をOBBのローカル座標系に変換する関数
	Vector3 SphereCenterToOBBLocal(const OBB& obb, const Vector3& sphereCenter);

public:

	//==============================================================================
	// accessor
	//==============================================================================

	//----- getter ---------------

	//ワールド座標の取得
	Vector3 GetWorldPos() override;
	//半径の取得
	float GetRadius() const { return radius_; }

	///----- setter ---------------
	
	/// 半径の設定
	void SetRadius(const float& radius) override { radius_ = radius; }

private:

	//球の半径
	float radius_;
};