#include "JsonLoader.h"
#include "Windows.h"
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

	json root;
	//json文字列からjsonのデータ構造に展開
	ifs >> root;
	//ファイルを閉じる
	ifs.close();

	//グループを検索
	json::iterator itGroup = root.find(groupName);
	//未登録の場合はassert
	assert(itGroup != root.end());

	//*   各アイテムについて   *//

	for(json::iterator itItem = itGroup->begin();
		itItem != itGroup->end(); ++itItem) {
	
		//アイテム名の取得
		const std::string& itemName = itItem.key();

		//=======================================
		//int32_t型の値を保持している場合
		//=======================================
		if (itItem->is_number_integer()) {
			//int32_t型の値を取得
			int32_t value = itItem->get<int32_t>();
			//グループに追加
			SetValue(groupName, itemName, value);
		}

		//=======================================
		// uint32_t型の値を保持している場合
		//=======================================
		else if (itItem->is_number_unsigned()) {
			//uint32_t型の値を取得
			uint32_t value = itItem->get<uint32_t>();
			//グループに追加
			SetValue(groupName, itemName, value);
		}

		//=======================================
		//float型の値を保持している場合
		//=======================================
		else if (itItem->is_number_float()) {
			//float型の値を取得
			double value = itItem->get<double>();
			//グループに追加
			SetValue(groupName, itemName, static_cast<float>(value));
		}

		//=======================================
		//要素数3の配列を保持している場合(Vector3)
		//=======================================
		else if (itItem->is_array() && itItem->size() == 3) {
			//float型のjson配列を取得
			Vector3 value = {itItem->at(0), itItem->at(1), itItem->at(2)};
			//グループに追加
			SetValue(groupName, itemName, value);
		}

		//=======================================
		//bool型の値を保持している場合
		//=======================================
		else if (itItem->is_boolean()) {
			//bool型の値を取得
			bool value = itItem->get<bool>();
			//グループに追加
			SetValue(groupName, itemName, value);
		}
	}
}

void JsonLoader::SaveParticlePreset(const std::string& presetName, const ParticleAttributes& attributes) {
	
	std::filesystem::path dirctory(kParticlePresetPath);
	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(dirctory)) {
		std::filesystem::create_directory(dirctory);
	}
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

void JsonLoader::to_json(json& j, const ParticleAttributes& attributes) const {

	j["scale"] = json::array({ attributes.scale.x, attributes.scale.y, attributes.scale.z });
	j["color"] = json::array({ attributes.color.x, attributes.color.y, attributes.color.z });

	j["positionRange"] = json::object({ {"min", attributes.positionRange.min, attributes.positionRange.max } });
	j["scaleRange"] = json::object({ {"min", attributes.scaleRange.min, attributes.scaleRange.max } });
	j["rotateRange"] = json::object({ {"min", attributes.rotateRange.min, attributes.rotateRange.max } });
	j["velocityRange"] = json::object({ {"min", attributes.velocityRange.min, attributes.velocityRange.max } });
	j["colorRange"] = json::object({ {"min", attributes.colorRange.min, attributes.colorRange.max } });
	j["lifetimeRange"] = json::object({ {"min", attributes.lifetimeRange.min, attributes.lifetimeRange.max } });
	j["editColor"] = attributes.editColor;
	j["isBillboard"] = attributes.isBillboard;
	j["scaleSetting_"] = attributes.scaleSetting_;

}
void JsonLoader::to_json(json& j, const AttributeRange& attributeRange) const {
	j = json{
		{"min",attributeRange.min},
		{"max",attributeRange.max}
	};
}
void JsonLoader::to_json(json& j, const Vector3& v) const {
	j = json{
		{"x", v.x},
		{"y", v.y},
		{"z", v.z}
	};
}

void JsonLoader::from_json(const json& j, ParticleAttributes& attributes) const {

	j.at("scale").get_to(attributes.scale);
}

void JsonLoader::from_json(const json& j, AttributeRange& attributeRange) const {

	j.at("min").get_to(attributeRange.min);
	j.at("max").get_to(attributeRange.max);
}
void JsonLoader::from_json(const json& j, Vector3& v) const {

	if (j.is_array() && j.size() == 3) {
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	} else {
		assert(false && "Invalid Vector3 JSON format");
	}
}
#pragma endregion