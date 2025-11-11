#include "RifleInfo.h"

void to_json(nlohmann::json& jsonData, const RifleInfo& rifleInfo) {
	jsonData["burstShotInfo"] = rifleInfo.burstShotInfo; 
}

void from_json(const nlohmann::json& jsonData, RifleInfo& rifleInfo) {
	if (jsonData.contains("burstShotInfo"))jsonData.at("burstShotInfo").get_to(rifleInfo.burstShotInfo);
}
