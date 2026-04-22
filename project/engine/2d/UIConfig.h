#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

namespace TakeC {

	/// <summary>
	/// UISpriteConfig - UIを構成する個別のスプライト設定
	/// </summary>
	struct UISpriteConfig {
		std::string key;               // UI内部での識別用キー (例: "BG", "Cursor")
		std::string spriteConfigPath; // 個別のSpriteConfig JSONへのパス (例: "PauseMenu_BG.json")
	};

	/// <summary>
	/// UIConfig - UI全体の設定をまとめた構造体
	/// </summary>
	struct UIConfig {
		std::string uiName;
		std::vector<UISpriteConfig> sprites;
	};

	// JSONシリアライズのマクロ定義
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UISpriteConfig,
		key,
		spriteConfigPath
	)

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UIConfig,
		uiName,
		sprites
	)
}

// UIConfigのJSONディレクトリパスを指定
TAKEC_DEFINE_JSON_DIRECTORY_PATH(TakeC::UIConfig, "Resources/JsonLoader/UIConfig/");
