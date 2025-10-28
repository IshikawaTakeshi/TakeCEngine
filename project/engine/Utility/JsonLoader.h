#pragma once
#include "engine/3d/Particle/ParticleAttribute.h"
#include "application/Entity/GameCharacterInfo.h"
#include "scene/LevelData.h"
#include <iostream>
#include <variant>
#include <string>
#include <map>
#include <json.hpp>

#include "Vector3.h"

//=============================================================================================
/// JsonLoader class
//=============================================================================================
class JsonLoader {
public:

	// JSON namespace alias
	using json = nlohmann::json;

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


	// ParticleAttributesの保存
	void SaveParticleAttribute(const std::string& presetName, const ParticleAttributes& attributes);
	// パーティクルプリセットの保存
	void SaveParticlePreset(const std::string& presetName, const ParticlePreset& preset);
	// ゲームキャラクターコンテキストの保存
	void SaveGameCharacterContext(const std::string& characterName, const GameCharacterContext& context);

	// ParticleAttributesの読み込み
	ParticleAttributes LoadParticleAttribute(const std::string& presetName) const;
	// パーティクルプリセットの読み込み
	ParticlePreset LoadParticlePreset(const std::string& presetName) const;
	// ゲームキャラクターコンテキストの読み込み
	GameCharacterContext LoadGameCharacterContext(const std::string& characterName) const;

	// パーティクルプリセットの削除
	void DeleteParticlePreset(const std::string& presetName);

	// パーティクルプリセットの一覧を取得
	std::vector<std::string> GetParticlePresetList() const;

	// パーティクルプリセットが存在するかチェック
	bool IsParticlePresetExists(const std::string& presetName) const;

private:

	//グローバル変数の保存先ファイルパス
	const std::string kDirectoryPath = "Resources/JsonLoader/";
	
	//パーティクルプリセットの保存先
	const std::string kParticlePresetPath = "Resources/JsonLoader/ParticlePresets/";

	//ゲームキャラクターコンテキストの保存先
	const std::string kGameCharacterContextPath = "Resources/JsonLoader/GameCharacters/";
};