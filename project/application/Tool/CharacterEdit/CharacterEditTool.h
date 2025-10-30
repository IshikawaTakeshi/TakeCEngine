#pragma once
#include <string>
#include <map>
#include <memory>

#include "application/Weapon/WeaponContext.h"
#include "application/Entity/WeaponUnit.h"
#include "application/Entity/GameCharacterInfo.h"
#include "engine/3d/Object3d.h"

//編集モード
enum class EditMode {
	CHARACTER_EDIT,
	WEAPON_EDIT
};;

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

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	void UpdateImGui();

	void UpdateCharacterEdit();
	void UpdateWeaponEdit();

	void Draw();
	void DrawUI();

	void CharacterDataSelect();

	WeaponData WeaponSelect(WeaponUnit unit);

	void SaveCharacterData(std::string characterName);
	void SaveWeaponData(std::string WeaponName);

	void LoadCharacterData(std::string characterName);
	void LoadWeaponData(std::string WeaponName);

	void LoadAllCharacterData();
	void LoadAllWeaponData();

private:

	std::unique_ptr<Object3d> previewCharacterModel_; //プレビュー用キャラクターモデル
	std::vector<std::unique_ptr<Object3d>> previewWeaponModels_; //プレビュー用武器モデル群

	//武器データマップ
	std::map<std::string, WeaponData> weaponDataMap_;
	//ゲームキャラクターコンテキストマップ
	std::map<std::string, GameCharacterContext> characterContextMap_;

	//編集中のキャラクターデータ
	CharacterData currentCharacterData_;

	std::vector<std::string> characterNames_; //キャラクター名リスト
	std::vector<std::string> weaponNames_; //武器名リスト

	// 上書き確認ダイアログ表示フラグ
	bool showOverwriteConfirm_ = false;
	// 上書き予定のキャラクター名
	std::string pendingCharacterName_;
	// 上書き予定の武器名
	std::string pendingWeaponName_;
};