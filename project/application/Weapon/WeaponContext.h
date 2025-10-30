#pragma once
#include <cstdint>
#include <string>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
//		WeaponData.h
//============================================================================

struct WeaponData {
	
	std::string weaponName = ""; //武器名
	std::string modelFilePath = ""; //モデルファイルパス

	float power = 0.0f; //武器の攻撃力
	float attackInterval = 0.0f; //攻撃間隔
	float bulletSpeed = 0.0f; //弾のスピード
	float effectiveRange = 0.0f; // 有効射程距離
	float maxReloadTime = 0.0f; // 最大リロード時間
	float requiredChargeTime = 0.0f; // 必要チャージ時間
	float homingRate = 0.0f;          // ホーミング率
	uint32_t maxBulletCount = 0; // 最大弾数

	bool canChargeAttack  = false; // チャージ攻撃フラグ
	bool canMoveShootable = false; // 移動撃ち可能か
	bool isStopShootOnly = false; // 停止撃ち専用か
};

// WeaponContextをJSONに変換する関数
void to_json(nlohmann::json& jsonData, const WeaponData& weaponContext);
// JSONからWeaponContextへデータを読み込む関数
void from_json(const nlohmann::json& jsonData, WeaponData& weaponContext);

//ディレクトリパス取得用テンプレート特殊化
template<>
struct JsonPath<WeaponData> {
	static std::filesystem::path GetDirectory() {
		return kGameCharacterContextPath;
	}
};