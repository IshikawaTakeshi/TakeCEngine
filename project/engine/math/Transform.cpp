#include "Transform.h"
#include "engine/base/ImGuiManager.h"

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

void EulerTransform::EditConfig(const std::string& windowName) {

	ImGui::SeparatorText(windowName.c_str());
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &rotate.x, 0.1f);
	ImGui::DragFloat3("Translate", &translate.x, 0.1f);
}

void QuaternionTransform::EditConfig(const std::string& windowName) {
	ImGui::SeparatorText(windowName.c_str());
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &rotate.x, 0.1f);
	ImGui::DragFloat3("Translate", &translate.x, 0.1f);
}