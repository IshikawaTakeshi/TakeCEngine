#pragma once
#include <string>
#include <map>
#include "application/Weapon/WeaponContext.h"
#include "application/Entity/WeaponUnit.h"
#include "application/Entity/GameCharacterInfo.h"

//============================================================================
//		CharacterEditTool class
//============================================================================
class CharacterEditTool {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	CharacterEditTool() = default;
	~CharacterEditTool() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void Update();

	void Draw();

	void CharacterDataSelect();

	WeaponData WeaponSelect(WeaponUnit unit);

	void SaveCharacterData();

	void LoadCharacterData();

private:

	//武器データマップ
	std::map<std::string, WeaponData> weaponDataMap_;
	//ゲームキャラクターコンテキストマップ
	std::map<std::string, GameCharacterContext> characterContextMap_;

	//編集中のキャラクターデータ
	CharacterData editCharacterData_;
};

