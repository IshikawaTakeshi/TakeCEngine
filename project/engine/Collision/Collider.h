#pragma once
#include "engine/3d/Object3d.h"
#include "engine/math/physics/Ray.h"
#include "engine/Collision/SurfaceType.h"
#include "engine/Collision/Capsule.h"
#include "engine/base/ComPtrAliasTemplates.h"
#include <cstdint>
#include <memory>

// 衝突レイヤーの列挙型
enum class CollisionLayer {
	None = 0,
	Player = 1 << 0,
	Enemy = 1 << 1,
	Bullet = 1 << 2,
	Level_Object = 1 << 3,
	Missile = 1 << 4,
	Sensor = 1 << 5,
	All = Player | Enemy | Bullet | Level_Object | Missile | Sensor,
	Ignoe = Player | Bullet | Enemy | Missile| Sensor, 
};

// 前方宣言
class Model;
class GameCharacter;

namespace TakeC {
	class DirectXCommon;
}

//=================================================================================
// Collider class
//=================================================================================
class Collider {
public:

	//=============================================================================
	// functions
	//=============================================================================

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Collider() = default;

	/// <summary>
	/// デストラクタ(仮想デストラクタ)
	/// </summary>
	virtual ~Collider() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="collisionObject"></param>
	virtual void Initialize(TakeC::DirectXCommon* dxCommon, Object3d* collisionObject) = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="collisionObject"></param>
	virtual void Update(Object3d* collisionObject) = 0;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	/// <param name="name"></param>
	virtual void UpdateImGui(const std::string& name) = 0;

	/// <summary>
	/// コライダーの描画
	/// </summary>
	virtual void DrawCollider() = 0;

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	virtual bool CheckCollision(Collider* other) = 0;

	/// <summary>
	/// レイとの交差判定
	/// </summary>
	virtual bool Intersects(const Ray& ray, RayCastHit& outHit) = 0;

	/// <summary>
	/// 球との交差判定
	/// </summary>
	virtual bool IntersectsSphere(const Ray& ray, float radius, RayCastHit& outHit) = 0;

	/// <summary>
	/// カプセルとの交差判定
	/// </summary>
	virtual bool IntersectsCapsule(const Capsule& capsule, RayCastHit& outHit) = 0;

public:

	//=============================================================================
	// accessor
	//=============================================================================

	//----- getter ---------------
	
	// ワールド座標の取得
	virtual Vector3 GetWorldPos() = 0;
	// ワールド行列の取得
	const Vector3& GetHalfSize() const { return halfSize_; }
	// オフセットの取得
	const Vector3& GetOffset() const { return offset_; }
	// 色の取得
	Vector4 GetColor() const;
	// 衝突半径の取得
	float GetRadius() const;
	// 種別IDの取得
	CollisionLayer GetCollisionLayerID();
	//SurfaceTypeの取得
	SurfaceType GetSurfaceType() const;
	/// 持ち主の取得
	GameCharacter* GetOwner() const { return owner_; }

	//----- setter ---------------

	/// 色設定
	void SetColor(const Vector4& color) { color_ = color; }
	/// オフセットの設定
	virtual void SetOffset(const Vector3& offset) { offset_ = offset; }
	/// 半径の設定
	virtual void SetRadius(const float& radius) { radius_ = radius; }
	/// 半分のサイズの設定
	virtual void SetHalfSize(const Vector3& halfSize) { halfSize_ = halfSize; }
	/// 種別IDの設定
	void SetCollisionLayerID(uint32_t typeID) { layerID_ = static_cast<CollisionLayer>(typeID); }
	/// 持ち主の設定
	void SetOwner(GameCharacter* owner) { owner_ = owner; }

protected:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	//DirectXCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;
	//持ち主へのポインタ
	GameCharacter* owner_ = nullptr;
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	//Transform
	EulerTransform transform_{};

	//カラー
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	//衝突時の情報
	SurfaceType surfaceType_;

	//オフセット
	Vector3 offset_ = { 0.0f,0.0f,0.0f };

	//衝突半径
	float radius_ = 1.0f;
	Vector3 halfSize_ = { 1.0f,1.0f,1.0f };
	//種別ID
	CollisionLayer layerID_ = CollisionLayer::None;
};