#pragma once
#include <string>
#include <json.hpp>
#include <array>
#include "engine/math/Vector3.h"
#include "engine/math/Transform.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include "application/Weapon/WeaponData.h"

// ステップブースト情報
struct StepBoostInfo {
	Vector3 direction = { 0.0f,0.0f,0.0f };    // ステップブーストの方向
	float speed = 0.0f;    // ステップブーストの速度
	float duration = 0.0f; // ステップブーストの持続時間
	float boostTimer = 0.0f;     // ステップブーストの持続タイマー
	float useEnergy = 0.0f;      // ステップブーストに必要なエネルギー
	//インターバル用
	float interval = 0.2f; // ステップブーストのインターバル
	float intervalTimer  = 0.0f; // ステップブーストのインターバルタイマー
};

// ジャンプ情報
struct JumpInfo {
	float speed = 50.0f;        // ジャンプの速度
	float jumpTimer = 0.0f;               // ジャンプのタイマー
	float maxJumpTime = 0.5f;       // ジャンプの最大時間
	float deceleration = 40.0f; // ジャンプ中の減速率
	float useEnergy = 150.0f;         // ジャンプに必要なエネルギー
	bool isJumping = false; // ジャンプするかどうか
};

// チャージ攻撃後の硬直情報
struct ChargeAttackStunInfo {
	float stunTimer = 0.0f;    //チャージ攻撃後の硬直時間
	float stunDuration = 0.5f; // チャージ攻撃後の硬直時間
	bool isStunned = false; // チャージ攻撃後に硬直しているかどうか
};

// エネルギー情報
struct EnergyInfo {
	float energy = 0.0f;               // 現在のエネルギー
	float maxEnergy = 1000.0f;         // 最大エネルギー
	float recoveryRate = 200.0f;    // エネルギーの回復速度
	float energyCooldown = 1.0f; // エネルギー使用後のクールダウン時間
	bool isEnergyDepleted = false; // エネルギーが枯渇しているかどうか
};

// オーバーヒート情報
struct OverHeatInfo {
	float overheatTimer = 0.0f;          // オーバーヒートのタイマー
	float overheatDuration = 3.0f; // オーバーヒートの持続時間
	bool isOverheated = false;           // オーバーヒート中かどうか
};

//コライダーの情報（ボックスコライダー限定）
struct ColliderInfo {
	Vector3 offset{}; //コライダーのオフセット位置
	Vector3 halfSize{}; //コライダーの半径
};

// 操作可能なキャラクターの基礎情報
struct PlayableCharacterInfo {
	std::string characterName; //キャラクター名
	std::string modelFilePath; //モデルファイルパス
	QuaternionTransform transform{}; // 位置、回転、スケール
	Vector3 velocity{};              // 速度
	Vector3 moveDirection{};        //移動方向
	Vector3 focusTargetPos{};       // フォーカス対象の座標
	float deceleration = 1.1f;    //減速率
	float moveSpeed = 200.0f;     //移動速度
	float kMaxMoveSpeed = 120.0f; //移動速度の最大値
	float fallSpeed = 40.0f;      //落下速度
	float health = 0.0f;          // 現在の体力
	float maxHealth = 10000.0f;   // 最大体力
	bool isAlive = true;          //生存しているかどうか
	bool onGround = true; // 地面にいるかどうか
	bool isChargeShooting = false; // チャージショット中かどうか
	bool isDamaged = false; // ダメージを受けたかどうか	
	bool isInCombat = false; // 戦闘状態フラグ

	StepBoostInfo stepBoostInfo{};   // ステップブースト情報
	JumpInfo jumpInfo{};             // ジャンプ情報
	ChargeAttackStunInfo chargeAttackStunInfo{}; // チャージ攻撃後の硬直情報
	EnergyInfo energyInfo{};         // エネルギー情報
	OverHeatInfo overHeatInfo{};     // オーバーヒート情報
	ColliderInfo colliderInfo{};   // コライダー情報
};

// 実際に使用するゲームキャラクターデータ
struct CharacterData {

	PlayableCharacterInfo characterInfo; // コンテキスト情報
	std::array<WeaponData, 4> weaponData; // 武器データ（最大4つ）
};

// JSON形式に変換
void to_json(nlohmann::json& j, const CharacterData& info);
void to_json(nlohmann::json& j, const PlayableCharacterInfo& info);

// JSON形式から変換
void from_json(const nlohmann::json& j, CharacterData& info);
void from_json(const nlohmann::json& j, PlayableCharacterInfo& info);

// JSONへの変換関数
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StepBoostInfo, direction, speed, duration, boostTimer, useEnergy, interval, intervalTimer)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JumpInfo, speed, jumpTimer, maxJumpTime, deceleration, useEnergy, isJumping)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChargeAttackStunInfo, stunTimer, stunDuration, isStunned)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnergyInfo, energy, maxEnergy, recoveryRate, energyCooldown, isEnergyDepleted)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OverHeatInfo, overheatTimer, overheatDuration, isOverheated)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ColliderInfo, offset, halfSize)

//データ保存先ディレクトリパスの設定
TAKEC_DEFINE_JSON_DIRECTORY_PATH(PlayableCharacterInfo, "Resources/JsonLoader/GameCharacters/");
TAKEC_DEFINE_JSON_DIRECTORY_PATH(CharacterData, "Resources/JsonLoader/GameCharacterData/");