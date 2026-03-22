#include "VerticalMissileInfo.h"
#include "engine/base/ImGuiManager.h"

void VerticalMissileInfo::EditConfigImGui() {
	ImGui::SliderFloat("maxAltitude", &maxAltitude, 0.0f, 500.0f);
	ImGui::SliderFloat("ascendSpeed", &ascendSpeed, 1.0f, 500.0f);
	ImGui::SliderFloat("explosionRadius", &explosionRadius, 1.0f, 50.0f);
	ImGui::SliderFloat("homingRateStart", &homingRateStart, 0.0f, 1.0f);
	ImGui::SliderFloat("homingRateEnd", &homingRateEnd, 0.0f, 1.0f);
	ImGui::SliderFloat("homingBlendDuration", &homingBlendDuration, 0.1f, 5.0f);
}

void from_json(const nlohmann::json& jsonData, VerticalMissileInfo& vmInfo) {
	vmInfo.maxAltitude = jsonData.value("maxAltitude", vmInfo.maxAltitude);
	vmInfo.ascendSpeed = jsonData.value("ascendSpeed", vmInfo.ascendSpeed);
	vmInfo.explosionRadius = jsonData.value("explosionRadius", vmInfo.explosionRadius);
	vmInfo.homingRateStart = jsonData.value("homingRateStart", vmInfo.homingRateStart);
	vmInfo.homingRateEnd = jsonData.value("homingRateEnd", vmInfo.homingRateEnd);
	vmInfo.homingBlendDuration = jsonData.value("homingBlendDuration", vmInfo.homingBlendDuration);
}

void to_json(nlohmann::json& jsonData, const VerticalMissileInfo& vmInfo) {
	jsonData["maxAltitude"] = vmInfo.maxAltitude;
	jsonData["ascendSpeed"] = vmInfo.ascendSpeed;
	jsonData["explosionRadius"] = vmInfo.explosionRadius;
	jsonData["homingRateStart"] = vmInfo.homingRateStart;
	jsonData["homingRateEnd"] = vmInfo.homingRateEnd;
	jsonData["homingBlendDuration"] = vmInfo.homingBlendDuration;
}