#include "VerticalMissileLauncherInfo.h"

void to_json(nlohmann::json& jsonData, const VerticalMissileLauncherInfo& vmLauncherInfo) {
	jsonData["burstShotInfo"] = vmLauncherInfo.burstShotInfo;
	jsonData["homingRate"] = vmLauncherInfo.homingRate;
}

void from_json(const nlohmann::json& jsonData, VerticalMissileLauncherInfo& vmLauncherInfo) {
	if (jsonData.contains("burstShotInfo"))jsonData.at("burstShotInfo").get_to(vmLauncherInfo.burstShotInfo);
	if (jsonData.contains("homingRate"))jsonData.at("homingRate").get_to(vmLauncherInfo.homingRate);
}