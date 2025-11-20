#pragma once
#include <string>

//==================================================================================
// JsonDirectoryPathData.h
//==================================================================================

// JSON namespace alias
using json = nlohmann::json;

template<typename T>
struct JsonPath {
	static std::string GetDirectory() {
		return "Resources/JsonLoader/"; // デフォルトのディレクトリ
	}
};

//グローバル変数の保存先ファイルパス
const std::string kDirectoryPath = "Resources/JsonLoader/";

//パーティクルプリセットの保存先
const std::string kParticlePresetPath = "Resources/JsonLoader/ParticlePresets/";

//ゲームキャラクターコンテキストの保存先
const std::string kGameCharacterContextPath = "Resources/JsonLoader/GameCharacters/";

const std::string kGameCharacterDataPath = "Resources/JsonLoader/GameCharacterData/";
//WeaponDataの保存先
const std::string kWeaponDataPath = "Resources/JsonLoader/WeaponConfig/";
//拡張ショット情報の保存先
const std::string kWeaponActionDataPath = "Resources/JsonLoader/WeaponConfig/Extra/";
//cameraDataの保存先
const std::string kCameraDataPath = "Resources/JsonLoader/Camera/";