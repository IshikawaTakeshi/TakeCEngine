#include "WeaponContext.h"

//============================================================================
// JSON <- WeaponData
//============================================================================
void to_json(nlohmann::json& jsonData, const WeaponData& weaponContext) {

	jsonData["weaponName"] = weaponContext.weaponName;
	jsonData["modelFilePath"] = weaponContext.modelFilePath;

	jsonData["power"] = weaponContext.power;
	jsonData["attackInterval"] = weaponContext.attackInterval;
	jsonData["bulletSpeed"] = weaponContext.bulletSpeed;
	jsonData["effectiveRange"] = weaponContext.effectiveRange;
	jsonData["maxReloadTime"] = weaponContext.maxReloadTime;
	jsonData["requiredChargeTime"] = weaponContext.requiredChargeTime;
	jsonData["homingRate"] = weaponContext.homingRate;
	jsonData["maxBulletCount"] = weaponContext.maxBulletCount;

	jsonData["canChargeAttack"] = weaponContext.canChargeAttack;
	jsonData["canMoveShootable"] = weaponContext.canMoveShootable;
	jsonData["isStopShootOnly"] = weaponContext.isStopShootOnly;
}

//============================================================================
// JSON -> WeaponData
//============================================================================

void from_json(const nlohmann::json& jsonData, WeaponData& weaponContext) {

	if (jsonData.contains("weaponName"))jsonData.at("weaponName").get_to(weaponContext.weaponName);
	if (jsonData.contains("modelFilePath"))jsonData.at("modelFilePath").get_to(weaponContext.modelFilePath);
	
	if (jsonData.contains("power"))jsonData.at("power").get_to(weaponContext.power);
	if (jsonData.contains("attackInterval"))jsonData.at("attackInterval").get_to(weaponContext.attackInterval);
	if (jsonData.contains("bulletSpeed"))jsonData.at("bulletSpeed").get_to(weaponContext.bulletSpeed);
	if (jsonData.contains("effectiveRange"))jsonData.at("effectiveRange").get_to(weaponContext.effectiveRange);
	if (jsonData.contains("maxReloadTime"))jsonData.at("maxReloadTime").get_to(weaponContext.maxReloadTime);
	if (jsonData.contains("requiredChargeTime"))jsonData.at("requiredChargeTime").get_to(weaponContext.requiredChargeTime);
	if (jsonData.contains("homingRate"))jsonData.at("homingRate").get_to(weaponContext.homingRate);
	if (jsonData.contains("maxBulletCount"))jsonData.at("maxBulletCount").get_to(weaponContext.maxBulletCount);

	if (jsonData.contains("canChargeAttack"))jsonData.at("canChargeAttack").get_to(weaponContext.canChargeAttack);
	if (jsonData.contains("canMoveShootable"))jsonData.at("canMoveShootable").get_to(weaponContext.canMoveShootable);
	if (jsonData.contains("isStopShootOnly"))jsonData.at("isStopShootOnly").get_to(weaponContext.isStopShootOnly);
}