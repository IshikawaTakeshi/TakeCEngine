#pragma once
#include <json.hpp>

//============================================================================
// テクスチャアニメーションの種類
//============================================================================
namespace TakeC {

	enum class TextureAnimationType {
		None,           // アニメーションなし
		UVScroll,       // UVスクロール
		SpriteSheet,    // スプライトシート
	};

}

	void to_json(nlohmann::json& j, const TakeC::TextureAnimationType& type);
	void from_json(const nlohmann::json& j, TakeC::TextureAnimationType& type);