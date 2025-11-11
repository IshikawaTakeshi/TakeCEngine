#include "WeaponData.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"

//============================================================================
// JSON <- WeaponConfig
//============================================================================
void to_json(nlohmann::json& jsonData, const WeaponConfig& weaponContext) {

	jsonData["power"] = weaponContext.power;
	jsonData["kAttackInterval"] = weaponContext.kAttackInterval;
	jsonData["bulletSpeed"] = weaponContext.bulletSpeed;
	jsonData["effectiveRange"] = weaponContext.effectiveRange;
	jsonData["maxReloadTime"] = weaponContext.maxReloadTime;
	jsonData["requiredChargeTime"] = weaponContext.requiredChargeTime;
	jsonData["homingRate"] = weaponContext.homingRate;

	jsonData["magazineCount_"] = weaponContext.maxMagazineCount;
	jsonData["maxBulletCount"] = weaponContext.maxBulletCount;

	jsonData["canChargeAttack"] = weaponContext.canChargeAttack;
	jsonData["canMoveShootable"] = weaponContext.canMoveShootable;
	jsonData["isStopShootOnly"] = weaponContext.isStopShootOnly;
}

//============================================================================
// JSON <- WeaponData
//============================================================================

void to_json(nlohmann::json& jsonData, const WeaponData& weaponData) {
	jsonData["weaponName"] = weaponData.weaponName;
	jsonData["modelFilePath"] = weaponData.modelFilePath;
	jsonData["weaponType"] = StringUtility::EnumToString(weaponData.weaponType);
	jsonData["config"] = weaponData.config;
	
	if(weaponData.weaponType == WeaponType::WEAPON_TYPE_RIFLE) {
		jsonData["actionData"] = std::get<RifleInfo>(weaponData.actionData);
	} else if(weaponData.weaponType == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
		jsonData["actionData"] = std::get<VerticalMissileLauncherInfo>(weaponData.actionData);
	}
}

//============================================================================
// JSON -> WeaponConfig
//============================================================================

void from_json(const nlohmann::json& jsonData, WeaponConfig& weaponContext) {

	if (jsonData.contains("power"))jsonData.at("power").get_to(weaponContext.power);
	if (jsonData.contains("kAttackInterval"))jsonData.at("kAttackInterval").get_to(weaponContext.kAttackInterval);
	if (jsonData.contains("bulletSpeed"))jsonData.at("bulletSpeed").get_to(weaponContext.bulletSpeed);
	if (jsonData.contains("effectiveRange"))jsonData.at("effectiveRange").get_to(weaponContext.effectiveRange);
	if (jsonData.contains("maxReloadTime"))jsonData.at("maxReloadTime").get_to(weaponContext.maxReloadTime);
	if (jsonData.contains("requiredChargeTime"))jsonData.at("requiredChargeTime").get_to(weaponContext.requiredChargeTime);
	if (jsonData.contains("homingRate"))jsonData.at("homingRate").get_to(weaponContext.homingRate);

	if (jsonData.contains("magazineCount_"))jsonData.at("magazineCount_").get_to(weaponContext.maxMagazineCount);
	if (jsonData.contains("maxBulletCount"))jsonData.at("maxBulletCount").get_to(weaponContext.maxBulletCount);

	if (jsonData.contains("canChargeAttack"))jsonData.at("canChargeAttack").get_to(weaponContext.canChargeAttack);
	if (jsonData.contains("canMoveShootable"))jsonData.at("canMoveShootable").get_to(weaponContext.canMoveShootable);
	if (jsonData.contains("isStopShootOnly"))jsonData.at("isStopShootOnly").get_to(weaponContext.isStopShootOnly);
}

//============================================================================
// JSON -> WeaponData
//============================================================================
void from_json(const nlohmann::json& jsonData, WeaponData& weaponData) {

	if (jsonData.contains("weaponName"))jsonData.at("weaponName").get_to(weaponData.weaponName);
	if (jsonData.contains("modelFilePath"))jsonData.at("modelFilePath").get_to(weaponData.modelFilePath);
	if (jsonData.contains("weaponType")) {
		std::string weaponTypeStr;
		jsonData.at("weaponType").get_to(weaponTypeStr);
		weaponData.weaponType = StringUtility::StringToEnum<WeaponType>(weaponTypeStr);
	}
	if (jsonData.contains("config"))jsonData.at("config").get_to(weaponData.config);

	// actionDataの読み込み
	if (jsonData.contains("actionData")) {
		// weaponTypeに応じて適切な型で読み込む
		if (weaponData.weaponType == WeaponType::WEAPON_TYPE_RIFLE) {
			RifleInfo rifleInfo;
			jsonData.at("actionData").get_to(rifleInfo);
			weaponData.actionData = rifleInfo;
		}
		else if (weaponData.weaponType == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
			VerticalMissileLauncherInfo vmLauncherInfo;
			jsonData.at("actionData").get_to(vmLauncherInfo);
			weaponData.actionData = vmLauncherInfo;
		}
	}
}


//============================================================================
// configの編集
//============================================================================
void WeaponConfig::EditConfigImGui() {
	ImGui::SliderFloat("Power", &power, 0.0f, 500.0f);
	ImGui::SliderFloat("Attack Interval", &kAttackInterval, 0.01f, 2.0f);
	ImGui::SliderFloat("Bullet Speed", &bulletSpeed, 100.0f, 2000.0f);
	ImGui::SliderFloat("Effective Range", &effectiveRange, 50.0f, 2000.0f);
	ImGui::SliderFloat("Max Reload Time", &maxReloadTime, 0.1f, 10.0f);
	ImGui::SliderFloat("Required Charge Time", &requiredChargeTime, 0.0f, 5.0f);
	ImGui::SliderFloat("Homing Rate", &homingRate, 0.0f, 1.0f);
	ImGui::SliderInt("Max Magazine Count", reinterpret_cast<int*>(&maxMagazineCount), 1, 100);
	ImGui::SliderInt("Max Bullet Count", reinterpret_cast<int*>(&maxBulletCount), 1, 1000);
	ImGui::Checkbox("Can Charge Attack", &canChargeAttack);
	ImGui::Checkbox("Can Move Shootable", &canMoveShootable);
	ImGui::Checkbox("Is Stop Shoot Only", &isStopShootOnly);
}