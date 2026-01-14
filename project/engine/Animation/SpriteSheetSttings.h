#pragma once
#include <cstdint>
#include <string>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// スプライトシート設定
//============================================================================
struct SpriteSheetSettings {
	std::string filePath = ""; //JSONファイルパス
	uint32_t columns = 1;       // 横方向のフレーム数
	uint32_t rows = 1;          // 縦方向のフレーム数
	uint32_t totalFrames = 1;   // 総フレーム数
	float frameDuration = 0.1f; //アニメーション速度（1フレームあたりの時間）
	bool loop = true;           // ループするか

	void EditConfigImGui(const std::string& label);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteSheetSettings, columns, rows, totalFrames, frameDuration, loop)
TAKEC_DEFINE_JSON_DIRECTORY_PATH(SpriteSheetSettings, "Resources/JsonLoader/UVTextureAnimation/SpriteSheet/");