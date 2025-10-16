#include "ParticleAttribute.h"

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
}

void to_json(nlohmann::json& j, const AttributeRange& attributeRange) {

	j = json{
		{"min",attributeRange.min},
		{"max",attributeRange.max}
	};
}

void to_json(json& j, const ParticlePreset& preset) {

	j["name"] = preset.presetName;
	j["textureFilePath"] = preset.textureFilePath;
	j["blendState"] = preset.blendState;
	j["attributes"] = preset.attribute;
	j["primitiveType"] = preset.primitiveType;
	j["primitiveParameters"] = preset.primitiveParameters;
}

void from_json(const nlohmann::json& j, ParticleAttributes& attributes) {

	j.at("scale").get_to(attributes.scale);
	j.at("color").get_to(attributes.color);
	j.at("direction").get_to(attributes.direction);
	j.at("scaleRange").get_to(attributes.scaleRange);
	j.at("rotateRange").get_to(attributes.rotateRange);
	j.at("positionRange").get_to(attributes.positionRange);
	j.at("velocityRange").get_to(attributes.velocityRange);
	j.at("colorRange").get_to(attributes.colorRange);
	j.at("lifetimeRange").get_to(attributes.lifetimeRange);
	j.at("frequency").get_to(attributes.frequency);
	j.at("emitCount").get_to(attributes.emitCount);
	j.at("isBillboard").get_to(attributes.isBillboard);
	j.at("editColor").get_to(attributes.editColor);
	j.at("isTraslate").get_to(attributes.isTranslate);
	j.at("isDirectional").get_to(attributes.isDirectional);
	j.at("scaleSetting").get_to(attributes.scaleSetting);
	j.at("enableFollowEmitter").get_to(attributes.enableFollowEmitter);
}

void from_json(const nlohmann::json& j, AttributeRange& attributeRange) {

	j.at("min").get_to(attributeRange.min);
	j.at("max").get_to(attributeRange.max);
}

void from_json(const json& j, ParticlePreset& preset) {

	j.at("name").get_to(preset.presetName);
	j.at("textureFilePath").get_to(preset.textureFilePath);
	j.at("blendState").get_to(preset.blendState);
	j.at("attributes").get_to(preset.attribute);
	j.at("primitiveType").get_to(preset.primitiveType);
	j.at("primitiveParameters").get_to(preset.primitiveParameters);
}