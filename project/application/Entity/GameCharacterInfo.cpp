#include "GameCharacterInfo.h"

//============================================================================
// JSON <- GameCharacterInfo
//============================================================================

void to_json(nlohmann::json& j, const CharacterData& info) {
	
	
	j["info"] = info.characterInfo;
	j["weaponData"] = info.weaponData;
}

void to_json(nlohmann::json& j, const PlayableCharacterInfo& info) {
	j["characterName"] = info.characterName;
	j["modelFilePath"] = info.modelFilePath;
	j["transform"] = info.transform;

	j["velocity"] = info.velocity;
	j["moveDirection"] = info.moveDirection;
	j["focusTargetPos"] = info.focusTargetPos;

	j["deceleration"] = info.deceleration;
	j["moveSpeed"] = info.moveSpeed;
	j["kMaxMoveSpeed"] = info.kMaxMoveSpeed;
	j["fallSpeed"] = info.fallSpeed;
	j["health"] = info.health;
	j["maxHealth"] = info.maxHealth;

	j["isAlive"] = info.isAlive;
	j["onGround"] = info.onGround;
	j["isChargeShooting"] = info.isChargeShooting;
	j["isDamaged"] = info.isDamaged;
	j["isInCombat"] = info.isInCombat;

	j["stepBoostInfo"] = info.stepBoostInfo;
	j["jumpInfo"] = info.jumpInfo;
	j["chargeAttackStunInfo"] = info.chargeAttackStunInfo;
	j["energyInfo"] = info.energyInfo;
	j["overHeatInfo"] = info.overHeatInfo;
	j["colliderInfo"] = info.colliderInfo;
}


//============================================================================
// JSON <- CharacterData
//============================================================================

void from_json(const nlohmann::json& j, CharacterData& info) {

	if (j.contains("info"))j.at("info").get_to(info.characterInfo);
	if (j.contains("weaponData"))j.at("weaponData").get_to(info.weaponData);
}

void from_json(const nlohmann::json& j, PlayableCharacterInfo& info) {
	if (j.contains("characterName"))j.at("characterName").get_to(info.characterName);
	if (j.contains("modelFilePath"))j.at("modelFilePath").get_to(info.modelFilePath);
	if (j.contains("transform"))j.at("transform").get_to(info.transform);
	if (j.contains("velocity"))j.at("velocity").get_to(info.velocity);
	if (j.contains("moveDirection"))j.at("moveDirection").get_to(info.moveDirection);
	if (j.contains("focusTargetPos"))j.at("focusTargetPos").get_to(info.focusTargetPos);
	if (j.contains("deceleration"))j.at("deceleration").get_to(info.deceleration);
	if (j.contains("moveSpeed"))j.at("moveSpeed").get_to(info.moveSpeed);
	if (j.contains("kMaxMoveSpeed"))j.at("kMaxMoveSpeed").get_to(info.kMaxMoveSpeed);
	if (j.contains("fallSpeed"))j.at("fallSpeed").get_to(info.fallSpeed);
	if (j.contains("health"))j.at("health").get_to(info.health);
	if (j.contains("maxHealth"))j.at("maxHealth").get_to(info.maxHealth);

	if (j.contains("isAlive"))j.at("isAlive").get_to(info.isAlive);
	if (j.contains("onGround"))j.at("onGround").get_to(info.onGround);
	if (j.contains("isChargeShooting"))j.at("isChargeShooting").get_to(info.isChargeShooting);
	if (j.contains("isDamaged"))j.at("isDamaged").get_to(info.isDamaged);
	if (j.contains("isInCombat"))j.at("isInCombat").get_to(info.isInCombat);

	if (j.contains("stepBoostInfo"))j.at("stepBoostInfo").get_to(info.stepBoostInfo);
	if (j.contains("jumpInfo"))j.at("jumpInfo").get_to(info.jumpInfo);
	if (j.contains("chargeAttackStunInfo"))j.at("chargeAttackStunInfo").get_to(info.chargeAttackStunInfo);
	if (j.contains("energyInfo"))j.at("energyInfo").get_to(info.energyInfo);
	if (j.contains("overHeatInfo"))j.at("overHeatInfo").get_to(info.overHeatInfo);
	if (j.contains("colliderInfo"))j.at("colliderInfo").get_to(info.colliderInfo);
}