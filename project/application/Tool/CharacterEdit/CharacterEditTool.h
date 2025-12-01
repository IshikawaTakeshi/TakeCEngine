#pragma once
#include <string>
#include <map>
#include <memory>
#include <array>
#include <optional>

#include "application/Weapon/WeaponData.h"
#include "application/Entity/WeaponUnit.h"
#include "application/Entity/GameCharacterInfo.h"
#include "application/Tool/CharacterEdit/CharacterEditMenuEnum.h"
#include "engine/math/Vector2.h"
#include "engine/2d/Sprite.h"

//編集モード
enum class EditMode {
	EDIT_MENU,
	CHARACTER_EDIT,
	WEAPON_EDIT,
	NEXT_MENU
};

//前方宣言
class Object3d;

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

	void SelectEditItem();

	/// <summary>
	/// キャラクター編集更新処理
	/// </summary>
	void UpdateCharacterEdit();

	/// <summary>
	/// 武器編集更新処理
	/// </summary>
	void UpdateWeaponEdit();

	void DispatchObject();

	void DrawObject();

	void DrawUI();

	/// <summary>
	/// キャラクターデータ選択処理
	/// </summary>
	std::optional<PlayableCharacterInfo> AttachPlayableCharacterInfo(const std::string& characterName);

	/// <summary>
	/// 武器データ適用処理
	/// </summary>
	/// <param name="unit"></param>
	/// <returns></returns>
	std::optional<WeaponData> AttachWeapon(const std::string& weaponName);

	/// <summary>
	/// キャラクターデータ保存処理
	/// </summary>
	/// <param name="characterName"></param>
	void SavePlayableCharacterInfo(std::string characterName);

	/// <summary>
	/// 武器データ保存処理
	/// </summary>
	/// <param name="WeaponName"></param>
	void SaveWeaponData(std::string WeaponName,WeaponUnit unit);

	void SaveCharacterData();

	/// <summary>
	/// キャラクターデータ読み込み処理
	/// </summary>
	/// <param name="characterName"></param>
	void LoadPlayableCharacterInfo(std::string characterName);

	/// <summary>
	/// 武器データ読み込み処理
	/// </summary>
	/// <param name="WeaponName"></param>
	void LoadWeaponData(std::string WeaponName,WeaponUnit unit);

	/// <summary>
	/// 全キャラクターデータ・武器データ読み込み処理
	/// </summary>
	void LoadAllCharacterData();

	/// <summary>
	/// 全武器データ読み込み処理
	/// </summary>
	void LoadAllWeaponData();

public:

	//========================================================================
	// accessors
	//========================================================================
	//----- getter ---------------------------
	//編集モードリクエスト取得
	const std::optional<EditMode>& GetEditModeRequest() const { return editModeRequest_; }
	//編集中のキャラクターデータ取得
	const CharacterData& GetCurrentCharacterData() const { return currentCharacterData_; }
	//編集中のメニューアイテムインデックス取得
	const uint32_t& GetEditingItemIndex() const { return editingItemIndex_; }
	//メニューアイテム最大数取得
	bool IsNextMenuRequested() const { return nextMenuRequested_; }

	//----- setter ---------------------------
	//編集モードリクエスト設定
	void SetEditModeRequest(const std::optional<EditMode>& editModeRequest) { editModeRequest_ = editModeRequest; }
	//編集中のメニューアイテムインデックス設定
	void SetEditingItemIndex(const uint32_t& editingItemIndex) { editingItemIndex_ = editingItemIndex; }
	//次のメニューへ移動リクエストフラグ設定
	void SetNextMenuRequested(const bool& nextMenuRequested) { nextMenuRequested_ = nextMenuRequested; }

private:

	//プレビュー用キャラクターモデル
	std::unique_ptr<Object3d> previewCharacterModel_;
	//プレビュー用武器モデル群
	std::vector<std::unique_ptr<Object3d>> previewWeaponModels_;
	//メニューバースプライト群
	std::array<std::unique_ptr<Sprite>, CharacterEditMenuEnum::MENU_SIZE> menuBarSprites_;
	//カーソルスプライト
	std::unique_ptr<Sprite> cursorSprite_;
	//ゲーム開始テキストスプライト
	std::unique_ptr<Sprite> startGameTextSprite_;
	Vector2 startGameTextSpritePos_{ 400.0f,300.0f }; //ゲーム開始テキストスプライト座標

	std::vector<std::unique_ptr<Sprite>> weaponItemSprites_; //武器項目スプライトリスト
	std::vector<std::unique_ptr<Sprite>> weaponIconTexts_; //武器項目テキストスプライトリスト
	std::vector<std::unique_ptr<Sprite>> characterItemSprites_; //キャラクター項目スプライトリスト

	//メニューバースプライトサイズ
	Vector2 menuBarSpriteSize_{ 300.0f,50.0f }; 
	//メニューバー左上座標
	Vector2 menuBarLeftTop_{ 30.0f,30.0f }; 
	//メニューバーの縦間隔
	float menuBarSpacing_ = 10.0f; 
	//編集モードリクエスト
	std::optional<EditMode> editModeRequest_ = std::nullopt;
	//編集モード
	EditMode editMode_ = EditMode::CHARACTER_EDIT; 

	//編集中のキャラクターデータ
	CharacterData currentCharacterData_;
	//編集中のメニューアイテムインデックス
	uint32_t editingItemIndex_ = 0; 
	//メニューアイテム最大数（武器項目+キャラクター項目）
	const uint32_t maxMenuItems_ = CharacterEditMenuEnum::MENU_SIZE;

	//武器メニューアイテム最大数
	uint32_t maxWeaponMenuItems_ = 0;
	//編集中の武器ユニットインデックス
	uint32_t editingWeaponUnitIndex_ = 0;

	// 編集中のキャラクターインデックス
	uint32_t editingCharacterIndex_ = 0;
	// キャラクターメニューアイテム最大数
	uint32_t maxCharacterMenuItems_ = 0;

	//武器データマップ
	std::map<std::string, WeaponData> weaponDataMap_;
	//ゲームキャラクターコンテキストマップ
	std::map<std::string, PlayableCharacterInfo> playableCharacterInfoMap_;


	std::vector<std::string> characterNames_; //キャラクター名リスト
	std::vector<std::string> weaponNames_; //武器名リスト

	// 上書き確認ダイアログ表示フラグ
	bool showOverwriteConfirm_ = false;
	// 上書き予定のキャラクター名
	std::string pendingCharacterName_;
	// 上書き予定の武器名
	std::string pendingWeaponName_;

	// 次のメニューへ移動リクエストフラグ
	bool nextMenuRequested_ = false;
};