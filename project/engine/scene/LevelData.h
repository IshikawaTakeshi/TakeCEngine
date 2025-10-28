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
struct LevelData {

	// Boxコライダー構造体
	struct BoxCollider {
		Vector3 size; // コライダーのサイズ
	};
	// Sphereコライダー構造体
	struct SphereCollider {

		float radius; // コライダーの半径
	};

	// コライダー情報構造体
	struct ColliderData {
		std::string type; // コライダーの種類
		Vector3 center; // コライダーの中心位置
		std::variant<BoxCollider, SphereCollider> colliderData; // コライダーのデータ
		bool isValid; // 有効かどうか
	};

	// オブジェクトデータ構造体
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