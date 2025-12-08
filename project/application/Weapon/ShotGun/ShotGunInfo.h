#pragma once
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// ShotGunInfo struct
//============================================================================

struct ShotGunInfo {
	float spreadDeg = 15.0f; // 散布角度（度）
	int pelletCount = 10;    // ペレット数

	void EditConfigImGui();
};

template<>
struct JsonPath<ShotGunInfo> {
	static std::filesystem::path GetDirectory() {
		return kWeaponActionDataPath;
	}
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShotGunInfo, spreadDeg, pelletCount)