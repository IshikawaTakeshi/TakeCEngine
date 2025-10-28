#include "Transform.h"

void to_json(nlohmann::json& j, const EulerTransform& transform) {

	j["scale"] = transform.scale;
	j["rotate"] = transform.rotate;
	j["translate"] = transform.translate;
}

void to_json(nlohmann::json& j, const QuaternionTransform& transform) {
	j["scale"] = transform.scale;
	j["rotate"] = transform.rotate;
	j["translate"] = transform.translate;
}

void from_json(const nlohmann::json& j, EulerTransform& transform) {

	j.at("scale").get_to(transform.scale);
	j.at("rotate").get_to(transform.rotate);
	j.at("translate").get_to(transform.translate);
}

void from_json(const nlohmann::json& j, QuaternionTransform& transform) {
	j.at("scale").get_to(transform.scale);
	j.at("rotate").get_to(transform.rotate);
	j.at("translate").get_to(transform.translate);
}