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

// マクロ定義
#define TAKEC_DEFINE_JSON_DIRECTORY_PATH(Type, Directory) \
template<> \
struct JsonPath<Type> { \
    static std::string GetDirectory() { \
        return Directory; \
    } \
}

//グローバル変数の保存先ファイルパス
const std::string kDirectoryPath = "Resources/JsonLoader/";
//WeaponDataの保存先
const std::string kWeaponDataPath = "Resources/JsonLoader/WeaponConfig/";
//拡張ショット情報の保存先
const std::string kWeaponActionDataPath = "Resources/JsonLoader/WeaponConfig/Extra/";
//cameraDataの保存先
const std::string kCameraDataPath = "Resources/JsonLoader/Camera/";

