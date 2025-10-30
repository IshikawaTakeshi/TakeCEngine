#include "JsonLoader.h"
#include "Windows.h"

#include <fstream>
#include <cassert>
#include <imgui.h>

//===============================================================================================
///			レベルデータの読み込み
//===============================================================================================

LevelData* JsonLoader::LoadLevelFile(const std::string& groupName) {

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
		return nullptr;

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
			levelData->objects.emplace_back(LevelData::ObjectData{});
			LevelData::ObjectData& objectData = levelData->objects.back();
			objectData.name = object["name"].get<std::string>(); //名前を設定
			objectData.type = type; //種類を設定

			//モデルファイル名
			if (object.contains("file_name")) {
				objectData.file_name = object["file_name"].get<std::string>();
			}

			//コライダー情報
			if (object.contains("collider")) {
				const auto& colliderJson = object["collider"];
				objectData.collider.isValid = true;
				//type
				if (colliderJson.contains("type")) {
					objectData.collider.type = colliderJson["type"].get<std::string>();
				}

				//トランスフォーム情報
				nlohmann::json& transform = object["transform"];
				//translation
				objectData.translation = Vector3(
					transform["translation"][0].get<float>(),
					transform["translation"][2].get<float>(),
					transform["translation"][1].get<float>()
				);
				//rotation
				objectData.rotation = Vector3(
					-transform["rotation"][0].get<float>(),
					-transform["rotation"][2].get<float>(),
					-transform["rotation"][1].get<float>()
				);
				//scale
				objectData.scale = Vector3(
					transform["scaling"][0].get<float>(),
					transform["scaling"][2].get<float>(),
					transform["scaling"][1].get<float>()
				);

				//center
				if (colliderJson.contains("center")) {
					const auto& center = colliderJson["center"];
					objectData.collider.center = Vector3(
						center[0].get<float>(),
						center[2].get<float>(),
						center[1].get<float>()
					);
				}

				if (objectData.collider.type == "BOX") {
					//size
					if (colliderJson.contains("size")) {
						const auto& size = colliderJson["size"];
						objectData.collider.colliderData = LevelData::BoxCollider{
							Vector3(
								size[0].get<float>() * transform["scaling"][0].get<float>(),
								size[2].get<float>() * transform["scaling"][2].get<float>(),
								size[1].get<float>() * transform["scaling"][1].get<float>()
							)
						};
					}

				} else if (objectData.collider.type == "SPHERE") {
					//radius
					if (colliderJson.contains("radius")) {
						objectData.collider.colliderData = LevelData::SphereCollider{
							colliderJson["radius"].get<float>()* transform["scaling"][0].get<float>()
						};
					}
				} else {
					assert(0); //未対応のコライダータイプ
				}
			}

			
		}
	}

	return levelData;
}

//================================================================================================
///			パーティクルプリセットの削除
//================================================================================================

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

//================================================================================================
///			グループの作成
//================================================================================================

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

//================================================================================================
///			パーティクルプリセットが存在するかチェック
//================================================================================================

bool JsonLoader::IsParticlePresetExists(const std::string& presetName) const {

	std::string filePath = kParticlePresetPath + presetName + ".json";
	//ファイルが存在するかチェック
	return std::filesystem::exists(filePath);
}

