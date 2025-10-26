#pragma once
#include <json.hpp>

//============================================================================
// PrimitiveType enum
//============================================================================

//プリミティブタイプ列挙型
enum PrimitiveType {
	PRIMITIVE_RING,
	PRIMITIVE_PLANE,
	PRIMITIVE_SPHERE,
	PRIMITIVE_CUBE,
	PRIMITIVE_CONE,
	PRIMITIVE_CYLINDER,
	PRIMITIVE_COUNT
};

//jSON形式に変換
void to_json(nlohmann::json& j, const PrimitiveType& type);

//JSONから変換
void from_json(const nlohmann::json& j, PrimitiveType& type);