#pragma once
#include <filesystem>
#include <string>

#include "engine/Utility/ResourcePath.h"

//==================================================================================
// JsonDirectoryPathData.h
//==================================================================================

// JSON namespace alias
using json = nlohmann::json;

/// <summary>
/// 型ごとのJSON保存先ディレクトリを提供するテンプレート構造体です。
/// </summary>
template<typename T>
struct JsonPath {
	static std::filesystem::path GetDirectory() {
		return TakeC::ResourcePath::Game("JsonLoader");
	}
};

// マクロ定義
#define TAKEC_DEFINE_JSON_DIRECTORY_PATH(Type, Directory) \
/** <summary> \
 * 型ごとのJSON保存先ディレクトリを提供するテンプレート構造体です。 \
 * </summary> */ \
template<> \
struct JsonPath<Type> { \
    static std::filesystem::path GetDirectory() { \
        return TakeC::ResourcePath::Game(Directory); \
    } \
}

inline std::filesystem::path GetJsonDirectoryPath() {
	return TakeC::ResourcePath::Game("JsonLoader");
}

inline std::filesystem::path GetWeaponDataPath() {
	return TakeC::ResourcePath::Game("JsonLoader/WeaponConfig");
}

inline std::filesystem::path GetWeaponActionDataPath() {
	return TakeC::ResourcePath::Game("JsonLoader/WeaponConfig/Extra");
}

inline std::filesystem::path GetCameraDataPath() {
	return TakeC::ResourcePath::Game("JsonLoader/Camera");
}

