#pragma once
#include <iostream>
#include <variant>
#include <string>
#include <map>
#include <json.hpp>

#include "Vector3.h"

class JsonLoader {
public:

	using json = nlohmann::json;

	//シングルトンインスタンスの取得
	static JsonLoader* GetInstance();

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void CreateGroup(const std::string& groupName);

	/// <summary>
	/// ファイル書き出し
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void SaveFile(const std::string& groupName);

	/// <summary>
	/// ディレクトリの全ファイルの読み込み
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void LoadFile(const std::string& groupName);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/////////////////////////////////////////////////////////////////////////////////////////////
	///			getter
	/////////////////////////////////////////////////////////////////////////////////////////////

	//値の取得(int)
	int32_t GetIntValue(const std::string& groupName, const std::string& key) const;
	//値の取得(uint32_t)
	uint32_t GetUIntValue(const std::string& groupName, const std::string& key) const;
	//値の取得(float)
	float GetFloatValue(const std::string& groupName, const std::string& key) const;
	//値の取得(Vector3)
	Vector3 GetVector3Value(const std::string& groupName, const std::string& key) const;
	//値の取得(bool)
	bool GetBoolValue(const std::string& groupName, const std::string& key) const;

	/////////////////////////////////////////////////////////////////////////////////////////////
	///			setter
	/////////////////////////////////////////////////////////////////////////////////////////////

	//値のセット(int32_t)
	void SetValue(const std::string& groupName, const std::string& key, int32_t value);
	//値のセット(uint32_t)
	void SetValue(const std::string& groupName, const std::string& key, uint32_t value);
	//値のセット(float)
	void SetValue(const std::string& groupName, const std::string& key, float value);
	//値のセット(Vector3)
	void SetValue(const std::string& groupName, const std::string& key, const Vector3& value);
	//値のセット(bool)
	void SetValue(const std::string& groupName, const std::string& key, bool value);



	/////////////////////////////////////////////////////////////////////////////////////////////
	///			AddItem
	/////////////////////////////////////////////////////////////////////////////////////////////

	//項目の追加(int32_t)
	void AddItem(const std::string& groupName, const std::string& key, const int32_t& value);
	//項目の追加(uint32_t)
	void AddItem(const std::string& groupName, const std::string& key, const uint32_t& value);
	//項目の追加(float)
	void AddItem(const std::string& groupName, const std::string& key, const float& value);
	//項目の追加(Vector3)
	void AddItem(const std::string& groupName, const std::string& key, const Vector3& value);
	//項目の追加(bool)
	void AddItem(const std::string& groupName, const std::string& key, const bool& value);


private:

	JsonLoader() = default;
	~JsonLoader() = default;
	JsonLoader(const JsonLoader&) = delete;
	JsonLoader& operator=(const JsonLoader&) = delete;

private:

	//項目
	using Item = std::variant<int32_t,uint32_t, float, Vector3,bool>;
	//struct Item {
	//	std::variant<int32_t, float, Vector3> value;
	//};

	//グループ
	using Group = std::map<std::string, Item>;
	/*struct Group {
		std::map<std::string, Item> items;
	};*/

	//全データ
	std::map<std::string, Group> datas_;
	//グローバル変数の保存先ファイルパス
	const std::string kDirectoryPath = "Resources/JsonLoader/";
};

