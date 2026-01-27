#pragma once
#include <vector>
#include <string>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

struct EffectGroupConfig {

	std::string effectName;
	std::vector<std::string> particlePresetNames;
	bool isLoop;
};

void from_json(const nlohmann::json& j, EffectGroupConfig& config);

void to_json(nlohmann::json& j, const EffectGroupConfig& config);

TAKEC_DEFINE_JSON_DIRECTORY_PATH(EffectGroupConfig, "Resources/JsonLoader/EffectConfig/");