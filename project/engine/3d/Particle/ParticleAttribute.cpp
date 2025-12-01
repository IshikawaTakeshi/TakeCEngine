#include "ParticleAttribute.h"

//============================================================================
// JSON <- ParticleAttributes
//============================================================================
void to_json(nlohmann::json& j, const ParticleAttributes& attributes){

	j["scale"] = attributes.scale;
	j["color"] = attributes.color;
	j["direction"] = attributes.direction;

	j["positionRange"] = json{ {"min", attributes.positionRange.min},{"max", attributes.positionRange.max }};
	j["scaleRange"] = json{ {"min", attributes.scaleRange.min},{"max", attributes.scaleRange.max } };
	j["rotateRange"] = json{ {"min", attributes.rotateRange.min},{"max", attributes.rotateRange.max } };
	j["velocityRange"] = json{ { "min", attributes.velocityRange.min},{"max", attributes.velocityRange.max } };
	j["colorRange"] = json{ { "min", attributes.colorRange.min},{"max", attributes.colorRange.max } };
	j["lifetimeRange"] = json{ { "min", attributes.lifetimeRange.min },{"max", attributes.lifetimeRange.max } };
	j["frequency"] = attributes.frequency;
	j["emitCount"] = attributes.emitCount;
	j["editColor"] = attributes.editColor;
	j["isBillboard"] = attributes.isBillboard;
	j["scaleSetting"] = attributes.scaleSetting;
	j["isTraslate"] = attributes.isTranslate;
	j["isDirectional"] = attributes.isDirectional;
	j["enableFollowEmitter"] = attributes.enableFollowEmitter;

	j["isParticleTrail"] = attributes.isParticleTrail;
	j["particlesPerInterpolation"] = attributes.particlesPerInterpolation;
	j["trailEmitInterval"] = attributes.trailEmitInterval;
	j["isEmitterTrail"] = attributes.isEmitterTrail;
}

//============================================================================
// JSON <- AttributeRange
//============================================================================
void to_json(nlohmann::json& j, const AttributeRange& attributeRange) {

	j = json{
		{"min",attributeRange.min},
		{"max",attributeRange.max}
	};
}

//============================================================================
// JSON <- ParticlePreset
//============================================================================
void to_json(json& j, const ParticlePreset& preset) {

	j["name"] = preset.presetName;
	j["textureFilePath"] = preset.textureFilePath;
	j["blendState"] = preset.blendState;
	j["attributes"] = preset.attribute;
	j["primitiveType"] = preset.primitiveType;
	j["primitiveParameters"] = preset.primitiveParameters;
}

//============================================================================
// JSON -> ParticleAttributes
//============================================================================
void from_json(const nlohmann::json& j, ParticleAttributes& attributes) {

	if(j.contains("scale"))j.at("scale").get_to(attributes.scale);
	if(j.contains("color"))j.at("color").get_to(attributes.color);
	if(j.contains("direction"))j.at("direction").get_to(attributes.direction);
	if(j.contains("scaleRange"))j.at("scaleRange").get_to(attributes.scaleRange);
	if(j.contains("rotateRange"))j.at("rotateRange").get_to(attributes.rotateRange);
	if(j.contains("positionRange"))j.at("positionRange").get_to(attributes.positionRange);
	if(j.contains("velocityRange"))j.at("velocityRange").get_to(attributes.velocityRange);
	if(j.contains("colorRange"))j.at("colorRange").get_to(attributes.colorRange);
	if(j.contains("lifetimeRange"))j.at("lifetimeRange").get_to(attributes.lifetimeRange);
	if(j.contains("frequency"))j.at("frequency").get_to(attributes.frequency);
	if(j.contains("emitCount"))j.at("emitCount").get_to(attributes.emitCount);
	if(j.contains("isBillboard"))j.at("isBillboard").get_to(attributes.isBillboard);
	if(j.contains("editColor"))j.at("editColor").get_to(attributes.editColor);
	if(j.contains("isTraslate"))j.at("isTraslate").get_to(attributes.isTranslate);
	if(j.contains("isDirectional"))j.at("isDirectional").get_to(attributes.isDirectional);
	if(j.contains("scaleSetting"))j.at("scaleSetting").get_to(attributes.scaleSetting);
	if(j.contains("enableFollowEmitter"))j.at("enableFollowEmitter").get_to(attributes.enableFollowEmitter);
	if(j.contains("isParticleTrail"))j.at("isParticleTrail").get_to(attributes.isParticleTrail);
	if(j.contains("particlesPerInterpolation"))j.at("particlesPerInterpolation").get_to(attributes.particlesPerInterpolation);
	if(j.contains("trailEmitInterval"))j.at("trailEmitInterval").get_to(attributes.trailEmitInterval);
	if(j.contains("isEmitterTrail"))j.at("isEmitterTrail").get_to(attributes.isEmitterTrail);
}

//============================================================================
// JSON -> AttributeRange
//============================================================================
void from_json(const nlohmann::json& j, AttributeRange& attributeRange) {

	j.at("min").get_to(attributeRange.min);
	j.at("max").get_to(attributeRange.max);
}

//============================================================================
// JSON -> ParticlePreset
//============================================================================
void from_json(const json& j, ParticlePreset& preset) {

	j.at("name").get_to(preset.presetName);
	j.at("textureFilePath").get_to(preset.textureFilePath);
	j.at("blendState").get_to(preset.blendState);
	j.at("attributes").get_to(preset.attribute);
	j.at("primitiveType").get_to(preset.primitiveType);
	j.at("primitiveParameters").get_to(preset.primitiveParameters);
}