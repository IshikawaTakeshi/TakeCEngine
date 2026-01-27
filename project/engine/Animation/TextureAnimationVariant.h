#pragma once
#include "engine/Animation/SpriteSheetSttings.h"
#include "engine/Animation/UVScrollSettings.h"
#include <variant>

//============================================================================
// テクスチャアニメーションバリアント
//============================================================================

using TextureAnimationVariant = std::variant<
	std::monostate,
	UVScrollSettings,
	SpriteSheetSettings
>;