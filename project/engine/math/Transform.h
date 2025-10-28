#pragma once
#include <json.hpp>
#include "Vector3.h"
#include "Quaternion.h"

//=================================================================================
// Transform struct
//=================================================================================

// オイラー角による変換情報
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// クォータニオンによる変換情報
struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

//JSON形式に変換
void to_json(nlohmann::json& j, const EulerTransform& transform);
void to_json(nlohmann::json& j, const QuaternionTransform& transform);

//JSON形式から各構造体に変換
void from_json(const nlohmann::json& j, EulerTransform& transform);
void from_json(const nlohmann::json& j, QuaternionTransform& transform);