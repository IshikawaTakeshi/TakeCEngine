#include "PrimitiveType.h"

//============================================================================
// JSON <- PrimitiveType
//============================================================================
void to_json(nlohmann::json& j, const PrimitiveType& type) {

	switch (type) {
	case PRIMITIVE_RING:
		j = "PRIMITIVE_RING";
		break;
	case PRIMITIVE_PLANE:
		j = "PRIMITIVE_PLANE";
		break;
	case PRIMITIVE_SPHERE:
		j = "PRIMITIVE_SPHERE";
		break;
	case PRIMITIVE_CUBE:
		j = "PRIMITIVE_CUBE";
		break;
	case PRIMITIVE_CONE:
		j = "PRIMITIVE_CONE";
		break;
	case PRIMITIVE_CYLINDER:
		j = "PRIMITIVE_CYLINDER";
		break;
	default:
		j = "UNKNOWN";
		break;
	}
}

//============================================================================
// JSON -> PrimitiveType
//============================================================================
void from_json(const nlohmann::json& j, PrimitiveType& type) {

	std::string str = j.get<std::string>();
	if (str == "PRIMITIVE_RING") {
		type = PRIMITIVE_RING;
	} else if (str == "PRIMITIVE_PLANE") {
		type = PRIMITIVE_PLANE;
	} else if (str == "PRIMITIVE_SPHERE") {
		type = PRIMITIVE_SPHERE;
	} else if (str == "PRIMITIVE_CUBE") {
		type = PRIMITIVE_CUBE;
	} else if (str == "PRIMITIVE_CONE") {
		type = PRIMITIVE_CONE;
	} else if (str == "PRIMITIVE_CYLINDER") {
		type = PRIMITIVE_CYLINDER;
	} else {
		type = PRIMITIVE_COUNT; // 不明なタイプ
	}
}