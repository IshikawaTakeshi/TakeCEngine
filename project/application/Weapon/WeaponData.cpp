#include "WeaponData.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"

//============================================================================
// JSON <- WeaponConfig
//============================================================================
void to_json(nlohmann::json& jsonData, const WeaponConfig& weaponContext) {

	jsonData["power"] = weaponContext.power;
	jsonData["kAttackInterval"] = weaponContext.attackInterval;
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

	switch (weaponData.weaponType) {
	case WeaponType::WEAPON_TYPE_RIFLE:{
		const RifleInfo& rifleInfo = std::get<RifleInfo>(weaponData.actionData);
		jsonData["actionData"] = rifleInfo;
		break;
	}
	case WeaponType::WEAPON_TYPE_SHOTGUN:{
		const ShotGunInfo& shotGunInfo = std::get<ShotGunInfo>(weaponData.actionData);
		jsonData["actionData"] = shotGunInfo;
		break;
	}
	case WeaponType::WEAPON_TYPE_VERTICAL_MISSILE:{
		const VerticalMissileLauncherInfo& vmLauncherInfo = std::get<VerticalMissileLauncherInfo>(weaponData.actionData);
		jsonData["actionData"] = vmLauncherInfo;
		break;
	}
	}
}

//============================================================================
// JSON -> WeaponConfig
//============================================================================

void from_json(const nlohmann::json& jsonData, WeaponConfig& weaponContext) {

	if (jsonData.contains("power"))jsonData.at("power").get_to(weaponContext.power);
	if (jsonData.contains("kAttackInterval"))jsonData.at("kAttackInterval").get_to(weaponContext.attackInterval);
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
		switch (weaponData.weaponType) {
		case WeaponType::WEAPON_TYPE_RIFLE:
		{
			RifleInfo rifleInfo;
			jsonData.at("actionData").get_to(rifleInfo);
			weaponData.actionData = rifleInfo;
			break;
		}
		case WeaponType::WEAPON_TYPE_SHOTGUN:
		{
			ShotGunInfo shotGunInfo;
			jsonData.at("actionData").get_to(shotGunInfo);
			weaponData.actionData = shotGunInfo;
			break;
		}
		case WeaponType::WEAPON_TYPE_VERTICAL_MISSILE:
		{

			VerticalMissileLauncherInfo vmLauncherInfo;
			jsonData.at("actionData").get_to(vmLauncherInfo);
			weaponData.actionData = vmLauncherInfo;
			break;
		}
		}
	}
}


//============================================================================
// configの編集
//============================================================================
void WeaponConfig::EditConfigImGui(const std::string& weaponName) {
	ImGui::SeparatorText((weaponName + " Config").c_str());
	ImGui::SliderFloat(("Power##" + weaponName).c_str(), &power, 0.0f, 1000.0f);
	ImGui::SliderFloat(("Attack Interval##" + weaponName).c_str(), &attackInterval, 0.0f, 5.0f);
	ImGui::SliderFloat(("Bullet Speed##" + weaponName).c_str(), &bulletSpeed, 50.0f, 1000.0f);
	ImGui::SliderFloat(("Effective Range##" + weaponName).c_str(), &effectiveRange, 0.0f, 1000.0f);
	ImGui::SliderFloat(("Max Reload Time##" + weaponName).c_str(), &maxReloadTime, 0.0f, 10.0f);
	ImGui::SliderFloat(("Required Charge Time##" + weaponName).c_str(), &requiredChargeTime, 0.0f, 5.0f);
	ImGui::SliderFloat(("Homing Rate##" + weaponName).c_str(), &homingRate, 0.0f, 1.0f);
	ImGui::SliderInt(("Max Magazine Count##" + weaponName).c_str(), reinterpret_cast<int*>(&maxMagazineCount), 1, 100);
	ImGui::SliderInt(("Max Bullet Count##" + weaponName).c_str(), reinterpret_cast<int*>(&maxBulletCount), 1, 1000);
	ImGui::Checkbox(("Can Charge Attack##" + weaponName).c_str(), &canChargeAttack);
	ImGui::Checkbox(("Can Move Shootable##" + weaponName).c_str(), &canMoveShootable);
	ImGui::Checkbox(("Is Stop Shoot Only##" + weaponName).c_str(), &isStopShootOnly);
}