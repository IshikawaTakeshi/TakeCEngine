#pragma once
#include <json.hpp>
#include "Vector3.h"
#include "Quaternion.h"

//=================================================================================
// Transform struct
//=================================================================================

// オイラー角による変換情報
/// <summary>
/// EulerTransformの数学的な値と変換情報を表す構造体です。
/// </summary>
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

	void EditConfig(const std::string& windowName = "TransformEdit");
};

// クォータニオンによる変換情報
/// <summary>
/// QuaternionTransformの数学的な値と変換情報を表す構造体です。
/// </summary>
struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;

	void EditConfig(const std::string& windowName = "TransformEdit");
};

//JSON形式に変換
void to_json(nlohmann::json& j, const EulerTransform& transform);
void to_json(nlohmann::json& j, const QuaternionTransform& transform);

//JSON形式から各構造体に変換
void from_json(const nlohmann::json& j, EulerTransform& transform);
void from_json(const nlohmann::json& j, QuaternionTransform& transform);