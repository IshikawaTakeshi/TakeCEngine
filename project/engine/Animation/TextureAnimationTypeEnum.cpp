#include "TextureAnimationTypeEnum.h"

void to_json(nlohmann::json& j, const TextureAnimationType& type) {
	switch (type) {
	case TextureAnimationType::None:
		j = "None";
		break;
	case TextureAnimationType::UVScroll:
		j = "UVScroll";
		break;
	case TextureAnimationType::SpriteSheet:
		j = "SpriteSheet";
		break;
	default:
		j = "Unknown";
		break;
	}
}

void from_json(const nlohmann::json& j, TextureAnimationType& type) {
	std::string typeStr = j.get<std::string>();
	if (typeStr == "None") {
		type = TextureAnimationType::None;
	}
	else if (typeStr == "UVScroll") {
		type = TextureAnimationType::UVScroll;
	}
	else if (typeStr == "SpriteSheet") {
		type = TextureAnimationType::SpriteSheet;
	}
	else {
		type = TextureAnimationType::None; // デフォルト値
	}
}
