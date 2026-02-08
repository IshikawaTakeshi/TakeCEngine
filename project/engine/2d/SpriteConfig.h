#pragma once
#include "engine/math/Transform.h"
#include "engine/math/Vector2.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <string>
#include <json.hpp>

//=============================================================================
// SpriteConfig struct
//=============================================================================

struct SpriteConfig {
	std::string name = "DefaultSprite"; //スプライト名
	std::string jsonFilePath = ""; //JSONファイルパス
	std::string textureFilePath_ = "white1x1.png"; //テクスチャファイルパス
	Vector2 position_ = { 0.0f,0.0f };
	Vector2 size_ = { 400.0f,200.0f };
	Vector2 anchorPoint_ = { 0.0f,0.0f }; //アンカーポイント
	Vector2 textureLeftTop_ = { 0.0f,0.0f  }; //テクスチャの左上座標
	Vector2 textureSize_ = { 100.0f,100.0f }; //テクスチャの切り出しサイズ
	float rotation_ = 0.0f;

	void UpdateImGui();
};

//JSON形式に変換
void to_json(nlohmann::json& j, const SpriteConfig& spriteConfig);
//JSON形式から構造体に変換
void from_json(const nlohmann::json& j, SpriteConfig& spriteConfig);

TAKEC_DEFINE_JSON_DIRECTORY_PATH(SpriteConfig, "Resources/JsonLoader/SpriteConfig/");