#pragma once
#include "engine/3d/Particle/ParticleAttribute.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <variant>
#include <string>
#include <map>
#include <vector>
#include <json.hpp>
#include <typeinfo>

#include "Vector3.h"

//=============================================================================================
/// JsonLoader class
//=============================================================================================
namespace TakeC {
	/// <summary>
	/// JSONファイルからゲームデータを読み書きする機能を提供するクラスです。
	/// </summary>
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

		/// <summary>明示したディレクトリへJSONデータを保存します。</summary>
		template<typename T>
		void SaveJsonDataAt(const std::filesystem::path& directory, const std::string& filePath, const T& data);

		/// <summary>明示したディレクトリからJSONデータを読み込みます。</summary>
		template<typename T>
		T LoadJsonDataAt(const std::filesystem::path& directory, const std::string& filePath) const;

		/// <summary>明示したディレクトリにあるJSONデータを削除します。</summary>
		template<typename T>
		void DeleteJsonDataAt(const std::filesystem::path& directory, const std::string& filePath);

		/// <summary>明示したディレクトリにあるJSONファイル名一覧を取得します。</summary>
		template<typename T>
		std::vector<std::string> GetJsonDataListAt(const std::filesystem::path& directory) const;

		/// <summary>明示したディレクトリにJSONデータが存在するか確認します。</summary>
		template<typename T>
		bool IsJsonDataExistsAt(const std::filesystem::path& directory, const std::string& filePath) const;
	};

//-------------------------------------------------------------------------------
// JSONデータの保存
//-------------------------------------------------------------------------------
template<typename T>
	// フルパスを生成
inline void JsonLoader::SaveJsonData(const std::string& filePath, const T& data) {
	SaveJsonDataAt<T>(JsonPath<T>::GetDirectory(), filePath, data);
}

template<typename T>
inline void JsonLoader::SaveJsonDataAt(
	const std::filesystem::path& directory,
	const std::string& filePath,
	const T& data) {
	std::filesystem::path fileFullPath = directory / filePath;

	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(directory)) {
		std::filesystem::create_directories(directory);
	}
	// JSONオブジェクトに変換
	json presetJson = data;
	std::ofstream ofs(fileFullPath);
	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open json file for write:" + fileFullPath.string();
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
	return LoadJsonDataAt<T>(JsonPath<T>::GetDirectory(), filePath);
}

template<typename T>
inline T JsonLoader::LoadJsonDataAt(
	const std::filesystem::path& directory,
	const std::string& filePath) const {
	std::filesystem::path fileFullPath = directory / filePath;
	std::ifstream ifs(fileFullPath);
	//ファイルオープンが失敗した場合
	if (ifs.fail()) {
		std::string message = "Failed open json file for read:" + fileFullPath.string();
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
inline void JsonLoader::DeleteJsonData(const std::string& filePath) {
	DeleteJsonDataAt<T>(JsonPath<T>::GetDirectory(), filePath);
}

template<typename T>
inline void JsonLoader::DeleteJsonDataAt(
	const std::filesystem::path& directory,
	const std::string& filePath) {
	std::filesystem::path fileFullPath = directory / filePath;
	//ファイルが存在する場合
	if (std::filesystem::exists(fileFullPath)) {
		//ファイルを削除
		std::filesystem::remove(fileFullPath);
	} else {
		std::string message = "Json data not found: " + fileFullPath.string();
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
	}
}

//-------------------------------------------------------------------------------
// JSONデータリストの取得
//-------------------------------------------------------------------------------
template<typename T>
inline std::vector<std::string> JsonLoader::GetJsonDataList() const {
	return GetJsonDataListAt<T>(JsonPath<T>::GetDirectory());
}

template<typename T>
inline std::vector<std::string> JsonLoader::GetJsonDataListAt(
	const std::filesystem::path& directory) const {
	std::vector<std::string> dataList;
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
inline bool JsonLoader::IsJsonDataExists(const std::string& filePath) const {
	return IsJsonDataExistsAt<T>(JsonPath<T>::GetDirectory(), filePath);
}

template<typename T>
inline bool JsonLoader::IsJsonDataExistsAt(
	const std::filesystem::path& directory,
	const std::string& filePath) const {
	std::filesystem::path fileFullPath = directory / filePath;

	// ファイルの存在を返す
	return std::filesystem::exists(fileFullPath);
}

}
