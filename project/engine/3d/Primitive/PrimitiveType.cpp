#include "PrimitiveType.h"

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
	default:
		j = "UNKNOWN";
		break;
	}
}

void from_json(const nlohmann::json& j, PrimitiveType& type) {

	switch (type) {
	case PRIMITIVE_RING:
		if (j == "PRIMITIVE_RING") {
			type = PRIMITIVE_RING;
		}
	case PRIMITIVE_PLANE:
		if (j == "PRIMITIVE_PLANE") {
			type = PRIMITIVE_PLANE;
		}
	case PRIMITIVE_SPHERE:
		if (j == "PRIMITIVE_SPHERE") {
			type = PRIMITIVE_SPHERE;
		}
	default:
		type = PRIMITIVE_COUNT; //不明なタイプ
		break;
	}
}
