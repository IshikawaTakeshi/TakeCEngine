#pragma once
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// ShotGunInfo struct
//============================================================================

/// <summary>
/// ShotGunInfoに関するデータを保持する構造体です。
/// </summary>
struct ShotGunInfo {
	float spreadDeg = 15.0f; // 散布角度（度）
	int pelletCount = 10;    // ペレット数

	void EditConfigImGui();
};

/// <summary>
/// 型ごとのJSON保存先ディレクトリを提供するテンプレート構造体です。
/// </summary>
template<>
struct JsonPath<ShotGunInfo> {
	static std::filesystem::path GetDirectory() {
		return GetWeaponActionDataPath();
	}
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShotGunInfo, spreadDeg, pelletCount)
