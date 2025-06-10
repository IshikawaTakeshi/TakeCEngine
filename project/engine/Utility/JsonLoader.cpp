#include "JsonLoader.h"
#include "Windows.h"
#include "scene/LevelData.h"
#include <fstream>
#include <cassert>
#include <imgui.h>

//////////////////////////////////////////////////////////////////////////////////////
///			グループの作成
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::CreateGroup(const std::string& groupName) {

	//指定名のオブジェクトがなければ追加する
	datas_[groupName];

}

//////////////////////////////////////////////////////////////////////////////////////
///			ファイル書き出し
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SaveFile(const std::string& groupName) {

	//グループを検索
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);
	//未登録の場合はassert
	assert(itGroup != datas_.end());

	json root;
	//データコンテナにグループの全データをまとめる
	root = json::object();
	//jsonオブジェクト登録
	root[groupName] = json::object();

	//各項目について
	for(std::map<std::string,Item>::iterator itItem = itGroup->second.begin();
		itItem != itGroup->second.end(); ++itItem) {

		//項目名の取得
		const std::string& itemName = itItem->first;
		//項目の参照を取得
		Item& item = itItem->second;

		//項目の型によって処理を分岐
		//=======================================================
		// int32_t型を保持している場合
		//=======================================================
		if (std::holds_alternative<int32_t>(item)) {

			//int32_t型の値を取得
			root[groupName][itemName] = std::get<int32_t>(item);
		}

		//=======================================================
		// uint32_t型を保持している場合
		//=======================================================
		else if (std::holds_alternative<uint32_t>(item)) {
		
			//uint32_t型の値を取得
			root[groupName][itemName] = std::get<uint32_t>(item);
		}

		//=======================================================
		// float型を保持している場合
		//=======================================================
		else if (std::holds_alternative<float>(item)) {

			//float型の値を取得
			root[groupName][itemName] = std::get<float>(item);
		}

		//=======================================================
		// Vector3型を保持している場合
		//=======================================================
		else if (std::holds_alternative<Vector3>(item)) {

			//Vector3型の値を取得
			Vector3 value = std::get<Vector3>(item);
			root[groupName][itemName] = json::array({ value.x,value.y,value.z });
		}

		//=======================================================
		// bool型を保持している場合
		//=======================================================
		else if (std::holds_alternative<bool>(item)) {

			//bool型の値を取得
			root[groupName][itemName] = std::get<bool>(item);
		}

	}

	//ディレクトリがなければ作成する
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	//書き込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	//書き込み用ファイルストリーム
	std::ofstream ofs;
	//ファイルを書き込み用に開く
	ofs.open(filePath);

	//ファイルオープン失敗？
	if (ofs.fail()) {
		//エラーメッセージを出力
		std::string message = "Failed open data file for write.";
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;
	}

	//ファイルにjson文字列を書き込む(インデント幅4)
	ofs << std::setw(4) << root << std::endl;
	//ファイルを閉じる
	ofs.close();
}

//////////////////////////////////////////////////////////////////////////////////////
///			ディレクトリの全ファイル読み込み
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::LoadFiles() {

	//保存先ディレクトリパス
	std::filesystem::path dir(kDirectoryPath);
	//ディレクトリがなければスキップする
	if (!std::filesystem::exists(dir)) return;

	//各ファイルの処理
	std::filesystem::directory_iterator dir_it(dir);
	for (const std::filesystem::directory_entry& entry : dir_it) {

		//ファイルパスの取得
		const std::filesystem::path& filePath = entry.path();
		//ファイル拡張子の取得
		std::string extension = filePath.extension().string();
		//拡張子が.jsonでなければスキップ
		if (extension.compare(".json") != 0) continue;

		//ファイル読み込み
		LoadFile(filePath.stem().string());
	}
}

//////////////////////////////////////////////////////////////////////////////////////
///			ファイル読み込み
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::LoadFile(const std::string& groupName) {

	//読み込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	//読み込み用ファイルストリーム
	std::ifstream ifs;
	//ファイルを読み込み用に開く
	ifs.open(filePath);

	//ファイルオープン失敗？
	if (ifs.fail()) {
		//エラーメッセージを出力
		std::string message = "Failed open data file for read.";
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;

	}

	// JSONファイルから解凍したデータ
	json deserializedJson;

	//解凍する
	ifs >> deserializedJson;

	//正しいレベルデータファイルかチェック
	assert(deserializedJson.is_object());
	assert(deserializedJson.contains("name"));
	assert(deserializedJson["name"].is_string());

	//"name"を文字列として取得
	std::string name = deserializedJson["name"].get<std::string>();
	//正しいレベルデータファイルかチェック
	assert(name.compare("scene") == 0);	

	//TODO:レベルデータ格納用インスタンスを生成
	LevelData* levelData = new LevelData();

	//"object"の全オブジェクト走査
	for (nlohmann::json& object : deserializedJson["objects"]) {
		assert(object.contains("type"));

		//種類を取得
		std::string type = object["type"].get<std::string>();

		//種類によって処理を分岐
		//MESH
		if (type.compare("MESH") == 0) {
			levelData->objects.emplace_back(Object3d());
			Object3d& object3d = levelData->objects.back();

			//モデルファイル名
			if (object.contains("file_name")) {
				object3d.SetModelFilePath(object["file_name"].get<std::string>());
			}

			//トランスフォーム情報
			nlohmann::json& transform = object["transform"];
			//translation
			object3d.SetTranslate(Vector3(
				transform["translation"][0].get<float>(),
				transform["translation"][1].get<float>(),
				transform["translation"][2].get<float>()
			));
			//rotation
			object3d.SetRotation(Vector3(
				-transform["rotation"][0].get<float>(),
				-transform["rotation"][1].get<float>(),
				-transform["rotation"][2].get<float>()
			));
			//scale
			object3d.SetScale(Vector3(
				transform["scale"][0].get<float>(),
				transform["scale"][1].get<float>(),
				transform["scale"][2].get<float>()
			));

		}
	}
}

//===============================================================================================
///			パーティクルプリセットの保存
//===============================================================================================

void JsonLoader::SaveParticlePreset(const std::string& presetName, const ParticleAttributes& attributes) {
	
	std::filesystem::path dirctory(kParticlePresetPath);
	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(dirctory)) {
		std::filesystem::create_directories(dirctory);
	}

	// JSONオブジェクトに変換
	json presetJson = attributes;
	std::string filePath = kParticlePresetPath + presetName + ".json";
	std::ofstream ofs(filePath);

	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open particle preset file for write:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;
	}

	// JSONファイルに書き込む
	ofs << std::setw(4) << presetJson << std::endl;
	// ファイルを閉じる
	ofs.close();
}

//===============================================================================================
///			パーティクルプリセットの読み込み
//===============================================================================================

ParticleAttributes JsonLoader::LoadParticlePreset(const std::string& presetName) const {
	
	std::string filePath = kParticlePresetPath + presetName + ".json";

	std::ifstream ifs(filePath);

	//ファイルオープンが失敗した場合
	if (ifs.fail()) {
		std::string message = "Failed open particle preset file for read:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return ParticleAttributes();
	}

	// JSONファイルから読み込む
	json presetJson;
	ifs >> presetJson;
	ifs.close();

	// JSONからParticleAttributesに変換
	return presetJson.get<ParticleAttributes>();
}

void JsonLoader::DeleteParticlePreset(const std::string& presetName) {

	std::string filePath = kParticlePresetPath + presetName + ".json";
	//ファイルが存在する場合
	if (std::filesystem::exists(filePath)) {
		//ファイルを削除
		std::filesystem::remove(filePath);
	} else {
		std::string message = "Particle preset not found: " + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
	}
}

std::vector<std::string> JsonLoader::GetParticlePresetList() const {
	
	std::vector<std::string> presetList;

	std::filesystem::path dir(kParticlePresetPath);
	//ディレクトリが存在しない場合は空のベクターを返す
	if (!std::filesystem::exists(dir)) {
		return presetList;
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		//ファイル拡張子が.jsonである場合
		if (entry.path().extension() == ".json") {
			//ファイル名を取得してリストに追加
			presetList.push_back(entry.path().stem().string());
		}
	}

	return presetList;
}

bool JsonLoader::IsParticlePresetExists(const std::string& presetName) const {
	
	std::string filePath = kParticlePresetPath + presetName + ".json";
	//ファイルが存在するかチェック
	return std::filesystem::exists(filePath);
}

//////////////////////////////////////////////////////////////////////////////////////
///			更新処理
//////////////////////////////////////////////////////////////////////////////////////

//void JsonLoader::Update() {
//
//	if (!ImGui::Begin("JsonLoader",nullptr,ImGuiWindowFlags_MenuBar)) {
//		ImGui::End();
//		return;
//	}
//	if (!ImGui::BeginMenuBar()) return;
//
//	//各グループについて
//	for (std::map<std::string, Group>::iterator itGroup = datas_.begin();
//		itGroup != datas_.end(); ++itGroup) {
//
//		//グループ名の取得
//		const std::string& groupName = itGroup->first;
//		//グループ名の参照を取得
//		Group& group = itGroup->second;
//
//		if (!ImGui::BeginMenu(groupName.c_str())) continue;
//
//		//各項目について
//		for (std::map<std::string, Item>::iterator itItem = group.begin();
//			itItem != group.end(); ++itItem) {
//
//			//項目名の取得
//			const std::string& itemName = itItem->first;
//			//項目の参照を取得
//			Item& item = itItem->second;
//
//			//項目の型によって処理を分岐
//			//=======================================================
//			// int32_t型を保持している場合
//			//=======================================================
//			if (std::holds_alternative<int32_t>(item)) {
//				int32_t* ptr = std::get_if<int32_t>(&item);
//				ImGui::SliderInt(itemName.c_str(), ptr,0,100);
//			}
//
//			//=======================================================
//			// uint32_t型を保持している場合
//			//=======================================================
//			else if (std::holds_alternative<uint32_t>(item)) {
//				uint32_t* ptr = std::get_if<uint32_t>(&item);
//				ImGui::SliderInt(itemName.c_str(), reinterpret_cast<int*>(ptr), 0, 100);
//			}
//
//			//=======================================================
//			// float型を保持している場合
//			//=======================================================
//			else if (std::holds_alternative<float>(item)) {
//				float* ptr = std::get_if<float>(&item);
//				ImGui::SliderFloat(itemName.c_str(), ptr,0.0f,100.0f);
//			}
//			
//			//=======================================================
//			// Vector3型を保持している場合
//			//=======================================================
//			else if (std::holds_alternative<Vector3>(item)) {
//				Vector3* ptr = std::get_if<Vector3>(&item);
//				ImGui::SliderFloat3(itemName.c_str(),reinterpret_cast<float*>(ptr),-10.0f,10.0f );
//			}
//
//			//=======================================================
//			// bool型を保持している場合
//			//=======================================================
//			else if (std::holds_alternative<bool>(item)) {
//				bool* ptr = std::get_if<bool>(&item);
//				ImGui::Checkbox(itemName.c_str(), ptr);
//			}
//		}
//
//		//改行
//		ImGui::Text("\n");
//		//セーブボタン
//		if (ImGui::Button("Save")) {
//			SaveFile(groupName);
//			std::string message = std::format("{}.json saved.", groupName);
//			MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
//		}
//
//		ImGui::EndMenu();
//	}
//
//	ImGui::EndMenuBar();
//	ImGui::End();
//
//}

#pragma region GetValue

//////////////////////////////////////////////////////////////////////////////////////
///			値の取得(int32_t)
//////////////////////////////////////////////////////////////////////////////////////

int32_t JsonLoader::GetIntValue(const std::string& groupName, const std::string& key) const {
	
	//指定グループが存在する場合assert
	assert(datas_.find(groupName) != datas_.end());

	//グループの参照の取得
	const Group& group = datas_.at(groupName);

	//指定グループに指定キーが存在する場合assert
	assert(group.find(key) != group.end());

	//指定キーの値を取得
	const Item& item = group.at(key);
	
	return std::get<int32_t>(item);
}

//////////////////////////////////////////////////////////////////////////////////////
///			値の取得(uint32_t)
//////////////////////////////////////////////////////////////////////////////////////


uint32_t JsonLoader::GetUIntValue(const std::string& groupName, const std::string& key) const {
	//指定グループが存在する場合assert
	assert(datas_.find(groupName) != datas_.end());

	//グループの参照の取得
	const Group& group = datas_.at(groupName);

	//指定グループに指定キーが存在する場合assert
	assert(group.find(key) != group.end());

	//指定キーの値を取得
	const Item& item = group.at(key);

	return std::get<uint32_t>(item);
}

//////////////////////////////////////////////////////////////////////////////////////
///			値の取得(float)
//////////////////////////////////////////////////////////////////////////////////////

float JsonLoader::GetFloatValue(const std::string& groupName, const std::string& key) const {
	
	//指定グループが存在する場合assert
	assert(datas_.find(groupName) != datas_.end());

	//グループの参照の取得
	const Group& group = datas_.at(groupName);

	//指定グループに指定キーが存在する場合assert
	assert(group.find(key) != group.end());

	//指定キーの値を取得
	const Item& item = group.at(key);

	return std::get<float>(item);
}

//////////////////////////////////////////////////////////////////////////////////////
///			値の取得(Vector3)
//////////////////////////////////////////////////////////////////////////////////////

Vector3 JsonLoader::GetVector3Value(const std::string& groupName, const std::string& key) const {
	
	//指定グループが存在する場合assert
	assert(datas_.find(groupName) != datas_.end());

	//グループの参照の取得
	const Group& group = datas_.at(groupName);

	//指定グループに指定キーが存在する場合assert
	assert(group.find(key) != group.end());

	//指定キーの値を取得
	const Item& item = group.at(key);

	return std::get<Vector3>(item);
}

//////////////////////////////////////////////////////////////////////////////////////
///			値の取得(bool)
//////////////////////////////////////////////////////////////////////////////////////

bool JsonLoader::GetBoolValue(const std::string& groupName, const std::string& key) const {

	//指定グループが存在する場合assert
	assert(datas_.find(groupName) != datas_.end());

	//グループの参照の取得
	const Group& group = datas_.at(groupName);

	//指定グループに指定キーが存在する場合assert
	assert(group.find(key) != group.end());

	//指定キーの値を取得
	const Item& item = group.at(key);

	return std::get<bool>(item);
}

#pragma endregion

#pragma region SetValue
//////////////////////////////////////////////////////////////////////////////////////
///			値のセット(int32_t)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SetValue(const std::string& groupName, const std::string& key, int32_t value) {

	//グループの参照の取得
	Group& group = datas_[groupName];
	//新しい項目のデータを追加
	Item newItem = value;
	//設定した項目をstd::mapに追加
	group[key] = newItem;
}

//////////////////////////////////////////////////////////////////////////////////////
///			値のセット(uint32_t)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SetValue(const std::string& groupName, const std::string& key, uint32_t value) {

	//グループの参照の取得
	Group& group = datas_[groupName];
	//新しい項目のデータを追加
	Item newItem = value;
	//設定した項目をstd::mapに追加
	group[key] = newItem;
}

//////////////////////////////////////////////////////////////////////////////////////
///			値のセット(float)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SetValue(const std::string& groupName, const std::string& key, float value) {

	//グループの参照の取得
	Group& group = datas_[groupName];
	//新しい項目のデータを追加
	Item newItem = value;
	//設定した項目をstd::mapに追加
	group[key] = newItem;

}

//////////////////////////////////////////////////////////////////////////////////////
///			値のセット(Vector3)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SetValue(const std::string& groupName, const std::string& key, const Vector3& value) {

	//グループの参照の取得
	Group& group = datas_[groupName];
	//新しい項目のデータを追加
	Item newItem = value;
	//設定した項目をstd::mapに追加
	group[key] = newItem;
}

//////////////////////////////////////////////////////////////////////////////////////
///			値のセット(bool)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::SetValue(const std::string& groupName, const std::string& key, bool value) {

	//グループの参照の取得
	Group& group = datas_[groupName];
	//新しい項目のデータを追加
	Item newItem = value;
	//設定した項目をstd::mapに追加
	group[key] = newItem;
}

#pragma endregion

#pragma region AddItem
//////////////////////////////////////////////////////////////////////////////////////
///			項目の追加(int32_t)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::AddItem(const std::string& groupName, const std::string& key, const int32_t& value) {

	//項目が未登録の場合
	if (datas_[groupName].find(key) == datas_[groupName].end()) {
		SetValue(groupName, key, value);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
///			項目の追加(uint32_t)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::AddItem(const std::string& groupName, const std::string& key, const uint32_t& value) {

	if (datas_[groupName].find(key) == datas_[groupName].end()) {
		SetValue(groupName, key, value);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
///			項目の追加(float)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::AddItem(const std::string& groupName, const std::string& key, const float& value) {

	//項目が未登録の場合
	if (datas_[groupName].find(key) == datas_[groupName].end()) {
		SetValue(groupName, key, value);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
///			項目の追加(Vector3)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::AddItem(const std::string& groupName, const std::string& key, const Vector3& value) {
	
	//項目が未登録の場合
	if (datas_[groupName].find(key) == datas_[groupName].end()) {
		SetValue(groupName, key, value);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
///			項目の追加(bool)
//////////////////////////////////////////////////////////////////////////////////////

void JsonLoader::AddItem(const std::string& groupName, const std::string& key, const bool& value) {

	//項目が未登録の場合
	if (datas_[groupName].find(key) == datas_[groupName].end()) {
		SetValue(groupName, key, value);
	}
}
#pragma endregion