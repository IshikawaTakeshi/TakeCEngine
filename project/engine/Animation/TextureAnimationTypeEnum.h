#pragma once
#include <json.hpp>

//============================================================================
// テクスチャアニメーションの種類
//============================================================================
enum class TextureAnimationType {
	None,           // アニメーションなし
	UVScroll,       // UVスクロール
	SpriteSheet,    // スプライトシート
};

void to_json(nlohmann::json& j, const TextureAnimationType& type);
void from_json(const nlohmann::json& j, TextureAnimationType& type);