#pragma once
#include "application/Weapon/Action/BurstShotInfo.h"
#include "application/Weapon/Bullet/VerticalMissileInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// VerticalMissileLauncherInfo struct
//============================================================================

struct VerticalMissileLauncherInfo {
	BurstShotInfo burstShotInfo{}; // バーストショット情報
	VerticalMissileInfo vmInfo{};  // 垂直ミサイル情報
	
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VerticalMissileLauncherInfo, burstShotInfo, vmInfo)

template<>
struct JsonPath<VerticalMissileLauncherInfo> {
	static std::filesystem::path GetDirectory() {
		return kWeaponActionDataPath;
	}
};