#pragma once
#include "Object3d.h"
#include <cstdint>
#include <memory>

class Collider {
public:

	virtual void Initialize(Object3dCommon* object3dCommon,const std::string& filePath) = 0;

	void Update();

	void DrawCollisionObj();

	/// <summary>
	/// 衝突時に呼ばれる関数
	/// </summary>
	virtual void OnCollision([[maybe_unused]] Collider* other) = 0;

	virtual bool CheckCollision(Collider* other) = 0;

public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	virtual Vector3 GetWorldPos() = 0;

	/// <summary>
	/// 衝突半径の取得
	///</summary>
	float GetRadius() const { return radius_; }


	/// <summary>
	/// 種別IDの取得
	/// </summary>
	uint32_t GetTypeID() const { return typeID_; }



public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// 半径の設定
	/// </summary>
	void SetRadius(float radius) { radius_ = radius; }

	/// <summary>
	/// 種別IDの設定
	/// </summary>
	void SetTypeID(uint32_t typeID) { typeID_ = typeID; }

protected:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	//衝突半径
	float radius_ = 0.9f;

	//種別ID
	uint32_t typeID_ = 0u;

	//衝突判定用オブジェクト
	std::unique_ptr<Object3d> collisionObject_;
};