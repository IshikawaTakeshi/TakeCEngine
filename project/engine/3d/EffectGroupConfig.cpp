#include "EffectGroupConfig.h"

//============================================================================
// JSON -> EffectGroupConfig
//============================================================================
void from_json(const nlohmann::json& j, EffectGroupConfig& config) {
	if (j.contains("effectName")) j.at("effectName").get_to(config.effectName);
	if (j.contains("particlePresetNames")) j.at("particlePresetNames").get_to(config.particlePresetNames);
	if (j.contains("isLoop")) j.at("isLoop").get_to(config.isLoop);
}

//============================================================================
// JSON <- EffectGroupConfig
//============================================================================
void to_json(nlohmann::json& j, const EffectGroupConfig& config) {

	j["effectName"] = config.effectName;
	j["particlePresetNames"] = config.particlePresetNames;
	j["isLoop"] = config.isLoop;
}