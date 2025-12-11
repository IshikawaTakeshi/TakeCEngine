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
namespace TakeC {
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

		/// <summary>
		/// JSONデータの削除
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="filePath"></param>
		template<typename T>
		void DeleteJsonData(const std::string& filePath);

		/// <summary>
		/// JSONデータリストの取得
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		std::vector<std::string> GetJsonDataList() const;

		/// <summary>
		/// JSONデータの存在チェック
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="filePath"></param>
		/// <returns></returns>
		template<typename T>
		bool IsJsonDataExists(const std::string& filePath) const;
	};
}

//-------------------------------------------------------------------------------
// JSONデータの保存
//-------------------------------------------------------------------------------
template<typename T>
inline void TakeC::JsonLoader::SaveJsonData(const std::string& filePath, const T& data) {
	// フルパスを生成
	std::filesystem::path directory = JsonPath<T>::GetDirectory();
	std::string fileFullPath = directory.string() + filePath;

	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(directory)) {
		std::filesystem::create_directories(directory);
	}
	// JSONオブジェクトに変換
	json presetJson = data;
	std::ofstream ofs(fileFullPath);
	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open json file for write:" + fileFullPath;
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
inline T TakeC::JsonLoader::LoadJsonData(const std::string& filePath) const {
	
	// フルパスを生成
	std::filesystem::path directory = JsonPath<T>::GetDirectory();
	std::string fileFullPath = directory.string() + filePath;
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

//-------------------------------------------------------------------------------
// JSONデータの削除
//-------------------------------------------------------------------------------
template<typename T>
inline void TakeC::JsonLoader::DeleteJsonData(const std::string& filePath) {

	std::filesystem::path directory = JsonPath<T>::GetDirectory();
	std::string fileFullPath = directory.string() + filePath;
	//ファイルが存在する場合
	if (std::filesystem::exists(fileFullPath)) {
		//ファイルを削除
		std::filesystem::remove(fileFullPath);
	} else {
		std::string message = "Json data not found: " + fileFullPath;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
	}
}

//-------------------------------------------------------------------------------
// JSONデータリストの取得
//-------------------------------------------------------------------------------
template<typename T>
inline std::vector<std::string> TakeC::JsonLoader::GetJsonDataList() const {
	
	std::vector<std::string> dataList;
	std::filesystem::path directory = JsonPath<T>::GetDirectory();
	//ディレクトリが存在しない場合は空のベクターを返す
	if (!std::filesystem::exists(directory)) {
		return dataList;
	}

	// JSONファイルを走査
	for (const auto& entry : std::filesystem::directory_iterator(directory)) {

		// 拡張子が.jsonのファイル名をリストに追加
		if (entry.path().extension() == ".json") {
			dataList.push_back(entry.path().stem().string());
		}
	}
	return dataList;
}

//-------------------------------------------------------------------------------
// JSONデータの存在チェック
//-------------------------------------------------------------------------------
template<typename T>
inline bool TakeC::JsonLoader::IsJsonDataExists(const std::string& filePath) const {
	
	// フルパスを生成して存在チェック
	std::filesystem::path directory = JsonPath<T>::GetDirectory();
	std::string fileFullPath = directory.string() + filePath;

	// ファイルの存在を返す
	return std::filesystem::exists(fileFullPath);
}
