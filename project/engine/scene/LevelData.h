#pragma once
#include "3d/Object3d.h"
#include <string>
#include <vector>
#include <memory>
#include <variant>

struct LevelData {

	struct BoxCollider {
		Vector3 size; // コライダーのサイズ
	};
	struct SphereCollider {

		float radius; // コライダーの半径
	};


	struct ColliderData {
		std::string type; // コライダーの種類
		Vector3 center; // コライダーの中心位置
		std::variant<BoxCollider, SphereCollider> colliderData; // コライダーのデータ
		bool isValid; // 有効かどうか
	};
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