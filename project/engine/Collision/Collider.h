#pragma once
#include "engine/3d/Object3d.h"
#include "engine/math/physics/Ray.h"
#include <cstdint>
#include <memory>

enum class CollisionLayer {
	None = 0,
	Player = 1 << 0,
	Enemy = 1 << 1,
	Bullet = 1 << 2,
	Level_Object = 1 << 3,
	All = Player | Enemy | Bullet | Level_Object,
	Ignoe = Player | Bullet | Enemy, // PlayerとBulletは衝突しない
};

class Model;
class DirectXCommon;
class GameCharacter;
class Collider {
public:

	virtual ~Collider() = default;

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	virtual void Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) = 0;

	virtual void Update(Object3d* collisionObject) = 0;

	virtual void UpdateImGui(const std::string& name) = 0;

	virtual void DrawCollider() = 0;

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	virtual bool CheckCollision(Collider* other) = 0;

	virtual bool Intersects(const Ray& ray, RayCastHit& outHit) = 0;


public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	virtual Vector3 GetWorldPos() = 0;

	Vector4 GetColor() const { return color_; }

	/// <summary>
	/// 衝突半径の取得
	///</summary>
	float GetRadius() const { return radius_; }


	/// <summary>
	/// 種別IDの取得
	/// </summary>
	CollisionLayer GetCollisionLayerID() const { return layerID_; }

public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// 色設定
	/// </summary>
	/// <param name="color"></param>
	void SetColor(const Vector4& color) { color_ = color; }

	/// <summary>
	/// 半径の設定
	/// </summary>
	virtual void SetRadius(const float& radius) { radius_ = radius; }

	virtual void SetHalfSize(const Vector3& halfSize) { halfSize_ = halfSize; }

	/// <summary>
	/// 種別IDの設定
	/// </summary>
	void SetCollisionLayerID(uint32_t typeID) { layerID_ = static_cast<CollisionLayer>(typeID); }

protected:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	//Transform
	EulerTransform transform_{};

	//
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	//衝突半径
	float radius_ = 1.0f;
	Vector3 halfSize_ = { 1.0f,1.0f,1.0f };
	//種別ID
	CollisionLayer layerID_ = CollisionLayer::None;
};