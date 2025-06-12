#pragma once
#include "Utility/JsonLoader.h"

enum PrimitiveType {
	PRIMITIVE_RING,
	PRIMITIVE_PLANE,
	PRIMITIVE_SPHERE,
	PRIMITIVE_COUNT
};

//jSON形式に変換
//void to_json(nlohmann::json& j, const PrimitiveType& type);