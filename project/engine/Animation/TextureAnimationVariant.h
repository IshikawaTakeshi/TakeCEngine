#pragma once
#include "engine/Animation/SpriteSheetSttings.h"
#include "engine/Animation/UVAnimationSettings.h"
#include <variant>

//============================================================================
// テクスチャアニメーションバリアント
//============================================================================

using TextureAnimationVariant = std::variant<UVScrollSettings, SpriteSheetSettings>;