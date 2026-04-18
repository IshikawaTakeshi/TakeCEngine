#pragma once
#include "application/Weapon/Action/BurstShotInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// RifleInfo struct
//============================================================================

// ライフル特有の情報構造体
struct RifleInfo {
	BurstShotInfo burstShotInfo; // バーストショット情報
};

void to_json(nlohmann::json& jsonData, const RifleInfo& rifleInfo);

void from_json(const nlohmann::json& jsonData, RifleInfo& rifleInfo);

template<>
struct JsonPath<RifleInfo> {
	static std::filesystem::path GetDirectory() {
		return kWeaponActionDataPath;
	}
};