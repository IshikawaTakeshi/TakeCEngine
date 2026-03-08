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

	j["stepBoostInfo"] = info.stepBoostInfo;
	j["jumpInfo"] = info.jumpInfo;
	j["chargeAttackStunInfo"] = info.chargeAttackStunInfo;
	j["energyInfo"] = info.energyInfo;
	j["overHeatInfo"] = info.overHeatInfo;
	j["colliderInfo"] = info.colliderInfo;
	j["breakGaugeInfo"] = info.breakGaugeInfo;
}


//============================================================================
// JSON <- CharacterData
//============================================================================

void from_json(const nlohmann::json& j, CharacterData& info) {

	if (j.contains("info"))j.at("info").get_to(info.characterInfo);
	if (j.contains("weaponData"))j.at("weaponData").get_to(info.weaponData);
}

void from_json(const nlohmann::json& j, PlayableCharacterInfo& info) {
	info.characterName = j.value("characterName", info.characterName);
	info.modelFilePath = j.value("modelFilePath", info.modelFilePath);
	info.transform = j.value("transform", info.transform);

	info.velocity = j.value("velocity", info.velocity);
	info.moveDirection = j.value("moveDirection", info.moveDirection);
	info.focusTargetPos = j.value("focusTargetPos", info.focusTargetPos);

	info.deceleration = j.value("deceleration", info.deceleration);
	info.moveSpeed = j.value("moveSpeed", info.moveSpeed);
	info.kMaxMoveSpeed = j.value("kMaxMoveSpeed", info.kMaxMoveSpeed);
	info.fallSpeed = j.value("fallSpeed", info.fallSpeed);
	info.health = j.value("health", info.health);
	info.maxHealth = j.value("maxHealth", info.maxHealth);

	info.isAlive = j.value("isAlive", info.isAlive);
	info.onGround = j.value("onGround", info.onGround);
	info.isChargeShooting = j.value("isChargeShooting", info.isChargeShooting);
	info.isDamaged = j.value("isDamaged", info.isDamaged);

	info.stepBoostInfo = j.value("stepBoostInfo", info.stepBoostInfo);
	info.jumpInfo = j.value("jumpInfo", info.jumpInfo);
	info.chargeAttackStunInfo = j.value("chargeAttackStunInfo", info.chargeAttackStunInfo);
	info.energyInfo = j.value("energyInfo", info.energyInfo);
	info.overHeatInfo = j.value("overHeatInfo", info.overHeatInfo);
	info.colliderInfo = j.value("colliderInfo", info.colliderInfo);
	info.breakGaugeInfo = j.value("breakGaugeInfo", info.breakGaugeInfo);
}