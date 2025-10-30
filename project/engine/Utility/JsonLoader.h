#pragma once
#include "engine/3d/Particle/ParticleAttribute.h"
#include "application/Entity/GameCharacterInfo.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include "scene/LevelData.h"
#include <iostream>
#include <variant>
#include <string>
#include <map>
#include <json.hpp>
#include <typeinfo>

#include "Vector3.h"

//=============================================================================================
/// JsonLoader class
//=============================================================================================

class JsonLoader {
public:

	//==========================================================================================
	// functions
	//==========================================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	JsonLoader() = default;
	~JsonLoader() = default;

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ名</param>
	LevelData* LoadLevelFile(const std::string& groupName);


	/// <summary>
	/// JSONデータの保存
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="filePath"></param>
	/// <param name="data"></param>
	template<typename T>
	void SaveJsonData(const std::string& filePath, const T& data);

	/// <summary>
	/// JSONデータの読み込み
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="filePath"></param>
	/// <returns></returns>
	template<typename T>
	T LoadJsonData(const std::string& filePath) const;

	// パーティクルプリセットの削除
	void DeleteParticlePreset(const std::string& presetName);

	// パーティクルプリセットの一覧を取得
	std::vector<std::string> GetParticlePresetList() const;

	// パーティクルプリセットが存在するかチェック
	bool IsParticlePresetExists(const std::string& presetName) const;
};


//-------------------------------------------------------------------------------
// JSONデータの保存
//-------------------------------------------------------------------------------
template<typename T>
inline void JsonLoader::SaveJsonData(const std::string& filePath, const T& data) {

	std::filesystem::path dirctory = JsonPath<T>::GetDirectory();
	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(dirctory)) {
		std::filesystem::create_directories(dirctory);
	}
	// JSONオブジェクトに変換
	json presetJson = data;
	std::ofstream ofs(filePath);
	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open json file for write:" + filePath;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;
	}
	// JSONファイルに書き込む
	ofs << presetJson.dump(4) << std::endl;
	// ファイルを閉じる
	ofs.close();
}

//-------------------------------------------------------------------------------
// JSONデータの読み込み
//-------------------------------------------------------------------------------
template<typename T>
inline T JsonLoader::LoadJsonData(const std::string& filePath) const {
	
	std::filesystem::path dirctory = JsonPath<T>::GetDirectory();
	std::string fileFullPath = dirctory.string() + filePath;
	std::ifstream ifs(fileFullPath);
	//ファイルオープンが失敗した場合
	if (ifs.fail()) {
		std::string message = "Failed open json file for read:" + fileFullPath;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return T();
	}
	// JSONファイルから読み込む
	json dataJson;
	ifs >> dataJson;
	ifs.close();
	// JSONからTに変換
	return dataJson.get<T>();
}