#include "BurstShotInfo.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/JsonLoader.h"

//============================================================================
// configの編集
//============================================================================
void BurstShotInfo::EditConfigImGui() {
	ImGui::SliderFloat("burstInterval", &kInterval, 0.01f, 1.0f);
	ImGui::SliderInt("burstCount", &kMaxBurstCount, 2, 10);
}

//============================================================================
// JSON <- BurstShotInfo
//============================================================================
void to_json(nlohmann::json& jsonData, const BurstShotInfo& burstShotInfo) {
	jsonData["kInterval"] = burstShotInfo.kInterval;
	jsonData["kMaxBurstCount"] = burstShotInfo.kMaxBurstCount;
}

//============================================================================
// JSON -> BurstShotInfo
//============================================================================
void from_json(const nlohmann::json& jsonData, BurstShotInfo& burstShotInfo) {
	jsonData.at("kInterval").get_to(burstShotInfo.kInterval);
	jsonData.at("kMaxBurstCount").get_to(burstShotInfo.kMaxBurstCount);
}