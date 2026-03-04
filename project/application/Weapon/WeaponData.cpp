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
	jsonData["breakStunPower"] = weaponContext.breakStunPower;
	jsonData["breakStunDecayInterval"] = weaponContext.breakStunDecayInterval;

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

	weaponContext.power = jsonData.value("power", weaponContext.power);
	weaponContext.attackInterval = jsonData.value("attackInterval", weaponContext.attackInterval);
	weaponContext.bulletSpeed = jsonData.value("bulletSpeed", weaponContext.bulletSpeed);
	weaponContext.effectiveRange = jsonData.value("effectiveRange", weaponContext.effectiveRange);
	weaponContext.maxReloadTime = jsonData.value("maxReloadTime", weaponContext.maxReloadTime);
	weaponContext.requiredChargeTime = jsonData.value("requiredChargeTime", weaponContext.requiredChargeTime);
	weaponContext.homingRate = jsonData.value("homingRate", weaponContext.homingRate);
	weaponContext.maxMagazineCount = jsonData.value("maxMagazineCount", weaponContext.maxMagazineCount);
	weaponContext.maxBulletCount = jsonData.value("maxBulletCount", weaponContext.maxBulletCount);
	weaponContext.canChargeAttack = jsonData.value("canChargeAttack", weaponContext.canChargeAttack);
	weaponContext.canMoveShootable = jsonData.value("canMoveShootable", weaponContext.canMoveShootable);
	weaponContext.isStopShootOnly = jsonData.value("isStopShootOnly", weaponContext.isStopShootOnly);

	weaponContext.breakStunPower = jsonData.value("breakStunPower", weaponContext.breakStunPower);
	weaponContext.breakStunDecayInterval = jsonData.value("breakStunDecayInterval", weaponContext.breakStunDecayInterval);
}

//============================================================================
// JSON -> WeaponData
//============================================================================
void from_json(const nlohmann::json& jsonData, WeaponData& weaponData) {

	weaponData.weaponName = jsonData.value("weaponName", weaponData.weaponName);
	weaponData.modelFilePath = jsonData.value("modelFilePath", weaponData.modelFilePath);
	weaponData.config = jsonData.value("config", weaponData.config);
	if (jsonData.contains("weaponType")) {
		std::string weaponTypeStr;
		jsonData.at("weaponType").get_to(weaponTypeStr);
		weaponData.weaponType = StringUtility::StringToEnum<WeaponType>(weaponTypeStr);
	}
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
	// ブレイクスタン関連のパラメータ
	ImGui::SeparatorText(("Break Stun##" + weaponName).c_str());
	ImGui::SliderFloat(("Break Stun Power##" + weaponName).c_str(), &breakStunPower, 0.0f, 100.0f);
	ImGui::SliderFloat(("Break Stun Decay Interval##" + weaponName).c_str(), &breakStunDecayInterval, 0.0f, 10.0f);
	// 弾薬関連のパラメータ
	ImGui::SeparatorText(("Ammo##" + weaponName).c_str());
	ImGui::SliderInt(("Max Magazine Count##" + weaponName).c_str(), reinterpret_cast<int*>(&maxMagazineCount), 1, 100);
	ImGui::SliderInt(("Max Bullet Count##" + weaponName).c_str(), reinterpret_cast<int*>(&maxBulletCount), 1, 1000);
	ImGui::Checkbox(("Can Charge Attack##" + weaponName).c_str(), &canChargeAttack);
	ImGui::Checkbox(("Can Move Shootable##" + weaponName).c_str(), &canMoveShootable);
	ImGui::Checkbox(("Is Stop Shoot Only##" + weaponName).c_str(), &isStopShootOnly);
}