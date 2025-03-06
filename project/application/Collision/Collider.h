#pragma once
#include "Object3d.h"
#include <cstdint>
#include <memory>

class Model;
class DirectXCommon;
class GameCharacter;
class Collider {
public:

	virtual ~Collider() = default;

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	virtual void Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) = 0;

	virtual void Update() = 0;

	virtual void DrawCollider() = 0;

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
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

	Model* GetModel() { return model_; }

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

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	//当たり判定のモデルのファイルパス
	std::string colliderFilePath_;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;
};