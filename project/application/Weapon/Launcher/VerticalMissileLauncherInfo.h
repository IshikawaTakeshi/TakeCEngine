#pragma once
#include "application/Weapon/Action/BurstShotInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// VerticalMissileLauncherInfo struct
//============================================================================

struct VerticalMissileLauncherInfo {
	BurstShotInfo burstShotInfo{}; // バーストショット情報
	float homingRate = 0.05f; // ホーミング率
};

void to_json(nlohmann::json& jsonData, const VerticalMissileLauncherInfo& vmLauncherInfo);

void from_json(const nlohmann::json& jsonData, VerticalMissileLauncherInfo& vmLauncherInfo);

template<>
struct JsonPath<VerticalMissileLauncherInfo> {
	static std::filesystem::path GetDirectory() {
		return kWeaponActionDataPath;
	}
};