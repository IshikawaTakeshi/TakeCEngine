#pragma once
#include "application/Weapon/Action/BurstShotInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// RifleInfo struct
//============================================================================

// ライフル特有の情報構造体
/// <summary>
/// RifleInfoに関するデータを保持する構造体です。
/// </summary>
struct RifleInfo {
	BurstShotInfo burstShotInfo; // バーストショット情報
};

void to_json(nlohmann::json& jsonData, const RifleInfo& rifleInfo);

void from_json(const nlohmann::json& jsonData, RifleInfo& rifleInfo);

/// <summary>
/// 型ごとのJSON保存先ディレクトリを提供するテンプレート構造体です。
/// </summary>
template<>
struct JsonPath<RifleInfo> {
	static std::filesystem::path GetDirectory() {
		return GetWeaponActionDataPath();
	}
};
