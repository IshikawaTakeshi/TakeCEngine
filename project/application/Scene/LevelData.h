#pragma once
#include "3d/Object3d.h"
#include <string>
#include <vector>
#include <memory>
#include <variant>

//============================================================================
//LevelData.h
//============================================================================

// レベルデータ構造体
/// <summary>
/// LevelDataに関するデータを保持する構造体です。
/// </summary>
struct LevelData {

	// Boxコライダー構造体
	/// <summary>
	/// 衝突判定で使用するBoxColliderの形状または判定結果を表す構造体です。
	/// </summary>
	struct BoxCollider {
		Vector3 size; // コライダーのサイズ
	};
	// Sphereコライダー構造体
	/// <summary>
	/// 衝突判定で使用するSphereColliderの形状または判定結果を表す構造体です。
	/// </summary>
	struct SphereCollider {

		float radius; // コライダーの半径
	};

	// コライダー情報構造体
	/// <summary>
	/// ColliderDataに関するデータを保持する構造体です。
	/// </summary>
	struct ColliderData {
		std::string type; // コライダーの種類
		Vector3 center; // コライダーの中心位置
		std::variant<BoxCollider, SphereCollider> colliderData; // コライダーのデータ
		bool isValid; // 有効かどうか
	};

	// オブジェクトデータ構造体
	/// <summary>
	/// ObjectDataに関するデータを保持する構造体です。
	/// </summary>
	struct ObjectData {
		std::string name; // オブジェクトの名前
		std::string type; // オブジェクトの種類
		std::string file_name; // モデルファイル名
		Vector3 translation; // 位置
		Vector3 rotation; // 回転
		Vector3 scale; // スケール

		ColliderData collider; // コライダー情報
	};


	std::vector<ObjectData> objects; // レベル内のオブジェクトのリスト
};