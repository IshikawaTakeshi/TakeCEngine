#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"
#include "application/Weapon/WeaponType.h"
#include "application/Weapon/Action/WeaponActionData.h"

//============================================================================
//		WeaponData.h
//============================================================================

// 武器データ構造体
struct WeaponConfig {

	float power = 0.0f; //武器の攻撃力
	float kAttackInterval = 0.25f; // 攻撃間隔定数
	float bulletSpeed = 0.0f; //弾のスピード
	float effectiveRange = 0.0f; // 有効射程距離
	float maxReloadTime = 0.0f; // 最大リロード時間
	float requiredChargeTime = 0.0f; // 必要チャージ時間
	float homingRate = 0.0f;          // ホーミング率
	uint32_t maxMagazineCount = 0; // マガジン内の最大弾数
	uint32_t maxBulletCount = 0; // 武器の最大弾数

	bool canChargeAttack  = false; // チャージ攻撃フラグ
	bool canMoveShootable = false; // 移動撃ち可能か
	bool isStopShootOnly = false; // 停止撃ち専用か

	//configの編集
	void EditConfigImGui();
};

// 武器の実行中の情報構造体
struct WeaponState {
	
	float attackInterval = 0.0f;       //攻撃間隔
	float reloadTime = 0.0f;           //現在のリロード時間
	float chargeTime = 0.0f;           // チャージ時間
	uint32_t bulletCount = 0;          // 現在の弾数
	uint32_t remainingBulletCount = 0; // 現在の残弾数
	bool isAvailable = true;           //使用可能か	
	bool isReloading = false;          //リロード中かどうか	
	bool isCharging = false;           // チャージ攻撃フラグ
};

struct WeaponData {
	std::string weaponName = ""; //武器名
	std::string modelFilePath = ""; //モデルファイルパス
	WeaponType weaponType = WeaponType::NONE; //武器の種類
	WeaponConfig config; //武器の基本データ
	WeaponDataVariant actionData; //武器の特殊アクションデータ
};

// WeaponContextをJSONに変換する関数
void to_json(nlohmann::json& jsonData, const WeaponConfig& weaponContext);
void to_json(nlohmann::json& jsonData, const WeaponData& weaponData);

// JSONからWeaponContextへデータを読み込む関数
void from_json(const nlohmann::json& jsonData, WeaponConfig& weaponContext);
void from_json(const nlohmann::json& jsonData, WeaponData& weaponData);

//ディレクトリパス取得用テンプレート特殊化
template<>
struct JsonPath<WeaponData> {
	static std::filesystem::path GetDirectory() {
		return kWeaponDataPath;
	}
};