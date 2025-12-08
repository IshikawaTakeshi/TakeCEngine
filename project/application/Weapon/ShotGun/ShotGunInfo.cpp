#include "ShotGunInfo.h"
#include "engine/Base/ImGuiManager.h"

void ShotGunInfo::EditConfigImGui() {
	ImGui::SliderFloat("Spread Degree", &spreadDeg, 0.0f, 90.0f, "%.1f");
	ImGui::SliderInt("Pellet Count", &pelletCount, 1, 50);
}