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