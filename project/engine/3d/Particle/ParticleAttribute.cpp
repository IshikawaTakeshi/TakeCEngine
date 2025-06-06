#include "ParticleAttribute.h"

void to_json(nlohmann::json& j, const ParticleAttributes& attributes){

	j["scale"] = json::array({ attributes.scale.x, attributes.scale.y, attributes.scale.z });
	j["color"] = json::array({ attributes.color.x, attributes.color.y, attributes.color.z });

	j["positionRange"] = json::object({ {"min", attributes.positionRange.min, attributes.positionRange.max } });
	j["scaleRange"] = json::object({ {"min", attributes.scaleRange.min, attributes.scaleRange.max } });
	j["rotateRange"] = json::object({ {"min", attributes.rotateRange.min, attributes.rotateRange.max } });
	j["velocityRange"] = json::object({ {"min", attributes.velocityRange.min, attributes.velocityRange.max } });
	j["colorRange"] = json::object({ {"min", attributes.colorRange.min, attributes.colorRange.max } });
	j["lifetimeRange"] = json::object({ {"min", attributes.lifetimeRange.min, attributes.lifetimeRange.max } });
	j["editColor"] = attributes.editColor;
	j["isBillboard"] = attributes.isBillboard;
	j["scaleSetting_"] = attributes.scaleSetting_;
}

void to_json(nlohmann::json& j, const AttributeRange& attributeRange) {

	j = json{
		{"min",attributeRange.min},
		{"max",attributeRange.max}
	};
}

void from_json(const nlohmann::json& j, ParticleAttributes& attributes) {

	j.at("scale").get_to(attributes.scale);
	j.at("color").get_to(attributes.color);
	j.at("scaleRange").get_to(attributes.scaleRange);
	j.at("rotateRange").get_to(attributes.rotateRange);
	j.at("angleRange").get_to(attributes.angleRange);
	j.at("positionRange").get_to(attributes.positionRange);
	j.at("velocityRange").get_to(attributes.velocityRange);
	j.at("colorRange").get_to(attributes.colorRange);
	j.at("lifetimeRange").get_to(attributes.lifetimeRange);
	j.at("frequency").get_to(attributes.frequency);
	j.at("emitCount").get_to(attributes.emitCount);
	j.at("isBillboard").get_to(attributes.isBillboard);
	j.at("editColor").get_to(attributes.editColor);
	j.at("isTraslate_").get_to(attributes.isTraslate_);
	j.at("scaleSetting_").get_to(attributes.scaleSetting_);
	j.at("enableFollowEmitter_").get_to(attributes.enableFollowEmitter_);
}

void from_json(const nlohmann::json& j, AttributeRange& attributeRange) {


	j.at("min").get_to(attributeRange.min);
	j.at("max").get_to(attributeRange.max);
}
