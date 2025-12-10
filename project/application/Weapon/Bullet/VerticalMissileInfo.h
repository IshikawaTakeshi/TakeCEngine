#pragma once
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// VerticalMissileLauncherInfo struct
//============================================================================

struct VerticalMissileInfo {
	//撃ちだされてからの上昇値
	float maxAltitude = 100.0f;
	//ミサイルの上昇速度
	float ascendSpeed = 140.0f;
	//ミサイルの爆発半径
	float explosionRadius = 5.0f;
	//ホーミングの度合い
	float homingRate = 0.1f;

	void EditConfigImGui();
};

void to_json(nlohmann::json& jsonData, const VerticalMissileInfo& vmInfo);
void from_json(const nlohmann::json& jsonData, VerticalMissileInfo& vmInfo);