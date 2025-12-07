#include "VerticalMissileInfo.h"
#include "engine/base/ImGuiManager.h"

void VerticalMissileInfo::EditConfigImGui() {
	ImGui::SliderFloat("maxAltitude", &maxAltitude, 0.0f, 500.0f);
	ImGui::SliderFloat("ascendSpeed", &ascendSpeed, 1.0f, 500.0f);
	ImGui::SliderFloat("explosionRadius", &explosionRadius, 1.0f, 50.0f);
	ImGui::SliderFloat("homingRate", &homingRate, 0.0f, 1.0f);
}

void from_json(const nlohmann::json& jsonData, VerticalMissileInfo& vmInfo) {
	if (jsonData.contains("maxAltitude")) jsonData.at("maxAltitude").get_to(vmInfo.maxAltitude);
	if (jsonData.contains("ascendSpeed")) jsonData.at("ascendSpeed").get_to(vmInfo.ascendSpeed);
	if (jsonData.contains("explosionRadius")) jsonData.at("explosionRadius").get_to(vmInfo.explosionRadius);
	if (jsonData.contains("homingRate")) jsonData.at("homingRate").get_to(vmInfo.homingRate);
}

void to_json(nlohmann::json& jsonData, const VerticalMissileInfo& vmInfo) {
	jsonData["maxAltitude"] = vmInfo.maxAltitude;
	jsonData["ascendSpeed"] = vmInfo.ascendSpeed;
	jsonData["explosionRadius"] = vmInfo.explosionRadius;
	jsonData["homingRate"] = vmInfo.homingRate;
}