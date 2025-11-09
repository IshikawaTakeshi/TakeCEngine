#include "WeaponContext.h"
#include "engine/base/ImGuiManager.h"

//============================================================================
// JSON <- WeaponData
//============================================================================
void to_json(nlohmann::json& jsonData, const WeaponData& weaponContext) {

	jsonData["weaponName"] = weaponContext.weaponName;
	jsonData["modelFilePath"] = weaponContext.modelFilePath;

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
// JSON -> WeaponData
//============================================================================

void from_json(const nlohmann::json& jsonData, WeaponData& weaponContext) {

	if (jsonData.contains("weaponName"))jsonData.at("weaponName").get_to(weaponContext.weaponName);
	if (jsonData.contains("modelFilePath"))jsonData.at("modelFilePath").get_to(weaponContext.modelFilePath);
	
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
// configの編集
//============================================================================
void WeaponData::EditConfigImGui() {
	ImGui::Text("Weapon Name", &weaponName);
	ImGui::Text("Model File Path", &modelFilePath);
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