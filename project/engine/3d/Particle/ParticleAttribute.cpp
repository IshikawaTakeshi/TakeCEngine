#include "ParticleAttribute.h"

using namespace TakeC;

//============================================================================
// JSON <- ParticleAttributes
//============================================================================
void to_json(nlohmann::json& j, const ParticleAttributes& attributes) {

	j["scale"] = attributes.scale;
	j["color"] = attributes.color;
	j["direction"] = attributes.direction;

	j["positionRange"] = json{ {"min", attributes.positionRange.min},{"max", attributes.positionRange.max } };
	j["scaleRange"] = json{ {"min", attributes.scaleRange.min},{"max", attributes.scaleRange.max } };
	j["scaleEasingType"] = attributes.scaleEasingType;
	j["rotateRange"] = json{ {"min", attributes.rotateRange.min},{"max", attributes.rotateRange.max } };
	j["velocityRange"] = json{ { "min", attributes.velocityRange.min},{"max", attributes.velocityRange.max } };
	j["velocityEasingType"] = attributes.velocityEasingType;
	j["colorRange"] = json{ { "min", attributes.colorRange.min},{"max", attributes.colorRange.max } };
	j["lifetimeRange"] = json{ { "min", attributes.lifetimeRange.min },{"max", attributes.lifetimeRange.max } };
	j["lifetimeEasingType"] = attributes.lifeTimeEasingType;
	j["frequency"] = attributes.frequency;
	j["emitCount"] = attributes.emitCount;
	j["editColor"] = attributes.editColor;
	j["isBillboard"] = attributes.isBillboard;
	j["scaleSetting"] = attributes.scaleSetting;
	j["isTraslate"] = attributes.isTranslate;
	j["isDirectional"] = attributes.isDirectional;
	j["enableLighting"] = attributes.enableLighting;
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

	std::visit([&j](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			j["primitiveParam"] = nullptr; // or any representation for monostate
		} else {
			j["primitiveParam"] = arg; // 利用可能な型の場合、そのままJSONに変換
		}
		}, preset.primitiveParam);

	j["isUseTextureAnimation"] = preset.isUseTextureAnimation;
	j["textureAnimationType"] = preset.textureAnimationType;
	std::visit([&j](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, UVScrollSettings>) {
			j["textureAnimationParam"] = arg;
		} else if constexpr (std::is_same_v<T, SpriteSheetSettings>) {
			j["textureAnimationParam"] = arg;
		} else {
			j["textureAnimationParam"] = nullptr; // or any representation for unsupported types
		}
		}, preset.textureAnimationParam);
}

//============================================================================
// JSON -> ParticleAttributes
//============================================================================
void from_json(const nlohmann::json& j, ParticleAttributes& attributes) {

	if (j.contains("scale"))j.at("scale").get_to(attributes.scale);
	if (j.contains("color"))j.at("color").get_to(attributes.color);
	if (j.contains("direction"))j.at("direction").get_to(attributes.direction);
	if (j.contains("scaleRange"))j.at("scaleRange").get_to(attributes.scaleRange);
	if (j.contains("scaleEasingType"))j.at("scaleEasingType").get_to(attributes.scaleEasingType);
	if (j.contains("rotateRange"))j.at("rotateRange").get_to(attributes.rotateRange);
	if (j.contains("positionRange"))j.at("positionRange").get_to(attributes.positionRange);
	if (j.contains("velocityRange"))j.at("velocityRange").get_to(attributes.velocityRange);
	if (j.contains("velocityEasingType"))j.at("velocityEasingType").get_to(attributes.velocityEasingType);
	if (j.contains("colorRange"))j.at("colorRange").get_to(attributes.colorRange);
	if (j.contains("lifetimeRange"))j.at("lifetimeRange").get_to(attributes.lifetimeRange);
	if (j.contains("lifetimeEasingType"))j.at("lifetimeEasingType").get_to(attributes.lifeTimeEasingType);
	if (j.contains("frequency"))j.at("frequency").get_to(attributes.frequency);
	if (j.contains("emitCount"))j.at("emitCount").get_to(attributes.emitCount);
	if (j.contains("isBillboard"))j.at("isBillboard").get_to(attributes.isBillboard);
	if (j.contains("editColor"))j.at("editColor").get_to(attributes.editColor);
	if (j.contains("isTraslate"))j.at("isTraslate").get_to(attributes.isTranslate);
	if (j.contains("isDirectional"))j.at("isDirectional").get_to(attributes.isDirectional);
	if (j.contains("scaleSetting"))j.at("scaleSetting").get_to(attributes.scaleSetting);
	if (j.contains("enableFollowEmitter"))j.at("enableFollowEmitter").get_to(attributes.enableFollowEmitter);
	if (j.contains("isParticleTrail"))j.at("isParticleTrail").get_to(attributes.isParticleTrail);
	if (j.contains("particlesPerInterpolation"))j.at("particlesPerInterpolation").get_to(attributes.particlesPerInterpolation);
	if (j.contains("trailEmitInterval"))j.at("trailEmitInterval").get_to(attributes.trailEmitInterval);
	if (j.contains("isEmitterTrail"))j.at("isEmitterTrail").get_to(attributes.isEmitterTrail);
	if (j.contains("enableLighting"))j.at("enableLighting").get_to(attributes.enableLighting);
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
	
	switch (preset.primitiveType) {
	case PRIMITIVE_RING:
	{
		RingParam ringParam;
		j.at("primitiveParam").get_to(ringParam);
		preset.primitiveParam = ringParam;
		break;
	}
	case PRIMITIVE_PLANE:
	{
		PlaneParam planeParam;
		j.at("primitiveParam").get_to(planeParam);
		preset.primitiveParam = planeParam;
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		SphereParam sphereParam;
		j.at("primitiveParam").get_to(sphereParam);
		preset.primitiveParam = sphereParam;
		break;
	}
	case PRIMITIVE_CUBE:
	{
		CubeParam cubeParam;
		j.at("primitiveParam").get_to(cubeParam);
		preset.primitiveParam = cubeParam;
		break;
	}
	case PRIMITIVE_CONE:
	{
		ConeParam coneParam;
		j.at("primitiveParam").get_to(coneParam);
		preset.primitiveParam = coneParam;
		break;
	}
	case PRIMITIVE_CYLINDER:
	{
		CylinderParam cylinderParam;
		j.at("primitiveParam").get_to(cylinderParam);
		preset.primitiveParam = cylinderParam;
		break;
	}
	default:
		break;
	}

	if (j.contains("isUseTextureAnimation"))j.at("isUseTextureAnimation").get_to(preset.isUseTextureAnimation);
	if (j.contains("textureAnimationType"))j.at("textureAnimationType").get_to(preset.textureAnimationType);
	if (j.contains("textureAnimationParam")) {
		switch (preset.textureAnimationType) {
		case TextureAnimationType::UVScroll:
		{
			UVScrollSettings uvScrollSettings;
			j.at("textureAnimationParam").get_to(uvScrollSettings);
			preset.textureAnimationParam = uvScrollSettings;
			break;
		}
		case TextureAnimationType::SpriteSheet:
		{
			SpriteSheetSettings spriteSheetSettings;
			j.at("textureAnimationParam").get_to(spriteSheetSettings);
			preset.textureAnimationParam = spriteSheetSettings;
			break;
		}
		default:
			break;
		}
	}
}