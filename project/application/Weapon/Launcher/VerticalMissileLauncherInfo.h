#pragma once
#include "application/Weapon/Action/BurstShotInfo.h"
#include "application/Weapon/Bullet/VerticalMissileInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// VerticalMissileLauncherInfo struct
//============================================================================

/// <summary>
/// VerticalMissileLauncherInfoに関するデータを保持する構造体です。
/// </summary>
struct VerticalMissileLauncherInfo {
	BurstShotInfo burstShotInfo{}; // バーストショット情報
	VerticalMissileInfo vmInfo{};  // 垂直ミサイル情報
	
};

// JSONへの変換関数
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VerticalMissileLauncherInfo, burstShotInfo, vmInfo)

/// <summary>
/// 型ごとのJSON保存先ディレクトリを提供するテンプレート構造体です。
/// </summary>
template<>
struct JsonPath<VerticalMissileLauncherInfo> {
	static std::filesystem::path GetDirectory() {
		return GetWeaponActionDataPath();
	}
};
