#include "GameCharacterInfo.h"

//============================================================================
// JSON <- GameCharacterInfo
//============================================================================

void to_json(nlohmann::json& j, const GameCharacterContext& context) {

	j["transform"] = context.transform;

	j["velocity"] = context.velocity;
	j["moveDirection"] = context.moveDirection;
	j["focusTargetPos"] = context.focusTargetPos;

	j["deceleration"] = context.deceleration;
	j["moveSpeed"] = context.moveSpeed;
	j["kMaxMoveSpeed"] = context.kMaxMoveSpeed;
	j["fallSpeed"] = context.fallSpeed;
	j["health"] = context.health;
	j["maxHealth"] = context.maxHealth;

	j["isAlive"] = context.isAlive;
	j["onGround"] = context.onGround;
	j["isChargeShooting"] = context.isChargeShooting;
	j["isDamaged"] = context.isDamaged;

	j["stepBoostInfo"] = context.stepBoostInfo;
	j["jumpInfo"] = context.jumpInfo;
	j["chargeAttackStunInfo"] = context.chargeAttackStunInfo;
	j["energyInfo"] = context.energyInfo;
	j["overHeatInfo"] = context.overHeatInfo;

	j["name"] = context.name;
}

void to_json(nlohmann::json& j, const StepBoostInfo& info) {

	j["Direction"] = info.direction;
	j["boostTimer"] = info.boostTimer;
	j["duration"] = info.duration;
	j["speed"] = info.speed;
	j["useEnergy"] = info.useEnergy;
	j["interval"] = info.interval;
	j["intervalTimer"] = info.intervalTimer;
}

void to_json(nlohmann::json& j, const JumpInfo& info) {
	j["speed"] = info.speed;
	j["jumpTimer"] = info.jumpTimer;
	j["maxJumpTime"] = info.maxJumpTime;
	j["deceleration"] = info.deceleration;
	j["useEnergy"] = info.useEnergy;
	j["isJumping"] = info.isJumping;
}

void to_json(nlohmann::json& j, const ChargeAttackStunInfo& info) {
	j["stunTimer"] = info.stunTimer;
	j["stunDuration"] = info.stunDuration;
	j["isStunned"] = info.isStunned;
}

void to_json(nlohmann::json& j, const EnergyInfo& info) {
	j["energy"] = info.energy;
	j["maxEnergy"] = info.maxEnergy;
	j["recoveryRate"] = info.recoveryRate;
	j["energyCooldown"] = info.energyCooldown;
	j["isEnergyDepleted"] = info.isEnergyDepleted;
}

void to_json(nlohmann::json& j, const OverHeatInfo& info) {
	j["overheatTimer"] = info.overheatTimer;
	j["overheatDuration"] = info.overheatDuration;
	j["isOverheated"] = info.isOverheated;
}


//============================================================================
// JSON <- GameCharacterInfo
//============================================================================

void from_json(const nlohmann::json& j, GameCharacterContext& context) {

	if (j.contains("transform"))j.at("transform").get_to(context.transform);
	if (j.contains("velocity"))j.at("velocity").get_to(context.velocity);
	if (j.contains("moveDirection"))j.at("moveDirection").get_to(context.moveDirection);
	if (j.contains("focusTargetPos"))j.at("focusTargetPos").get_to(context.focusTargetPos);
	if (j.contains("deceleration"))j.at("deceleration").get_to(context.deceleration);
	if (j.contains("moveSpeed"))j.at("moveSpeed").get_to(context.moveSpeed);
	if (j.contains("kMaxMoveSpeed"))j.at("kMaxMoveSpeed").get_to(context.kMaxMoveSpeed);
	if (j.contains("fallSpeed"))j.at("fallSpeed").get_to(context.fallSpeed);
	if (j.contains("health"))j.at("health").get_to(context.health);
	if (j.contains("maxHealth"))j.at("maxHealth").get_to(context.maxHealth);
	if (j.contains("isAlive"))j.at("isAlive").get_to(context.isAlive);
	if (j.contains("onGround"))j.at("onGround").get_to(context.onGround);
	if (j.contains("isChargeShooting"))j.at("isChargeShooting").get_to(context.isChargeShooting);
	if (j.contains("isDamaged"))j.at("isDamaged").get_to(context.isDamaged);
	if (j.contains("stepBoostInfo"))j.at("stepBoostInfo").get_to(context.stepBoostInfo);
	if (j.contains("jumpInfo"))j.at("jumpInfo").get_to(context.jumpInfo);
	if (j.contains("chargeAttackStunInfo"))j.at("chargeAttackStunInfo").get_to(context.chargeAttackStunInfo);
	if (j.contains("energyInfo"))j.at("energyInfo").get_to(context.energyInfo);
	if (j.contains("overHeatInfo"))j.at("overHeatInfo").get_to(context.overHeatInfo);
	if (j.contains("name"))j.at("name").get_to(context.name);

}

void from_json(const nlohmann::json& j, StepBoostInfo& info) {

	if (j.contains("Direction"))j.at("Direction").get_to(info.direction);
	if (j.contains("boostTimer"))j.at("boostTimer").get_to(info.boostTimer);
	if (j.contains("duration"))j.at("duration").get_to(info.duration);
	if (j.contains("speed"))j.at("speed").get_to(info.speed);
	if (j.contains("useEnergy"))j.at("useEnergy").get_to(info.useEnergy);
	if (j.contains("interval"))j.at("interval").get_to(info.interval);
	if (j.contains("intervalTimer"))j.at("intervalTimer").get_to(info.intervalTimer);
}

void from_json(const nlohmann::json& j, JumpInfo& info) {

	if (j.contains("speed"))j.at("speed").get_to(info.speed);
	if (j.contains("jumpTimer"))j.at("jumpTimer").get_to(info.jumpTimer);
	if (j.contains("maxJumpTime"))j.at("maxJumpTime").get_to(info.maxJumpTime);
	if (j.contains("deceleration"))j.at("deceleration").get_to(info.deceleration);
	if (j.contains("useEnergy"))j.at("useEnergy").get_to(info.useEnergy);
	if (j.contains("isJumping"))j.at("isJumping").get_to(info.isJumping);
}

void from_json(const nlohmann::json& j, ChargeAttackStunInfo& info) {
	if (j.contains("stunTimer"))j.at("stunTimer").get_to(info.stunTimer);
	if (j.contains("stunDuration"))j.at("stunDuration").get_to(info.stunDuration);
	if (j.contains("isStunned"))j.at("isStunned").get_to(info.isStunned);
}

void from_json(const nlohmann::json& j, EnergyInfo& info) {
	if (j.contains("energy"))j.at("energy").get_to(info.energy);
	if (j.contains("maxEnergy"))j.at("maxEnergy").get_to(info.maxEnergy);
	if (j.contains("recoveryRate"))j.at("recoveryRate").get_to(info.recoveryRate);
	if (j.contains("energyCooldown"))j.at("energyCooldown").get_to(info.energyCooldown);
	if (j.contains("isEnergyDepleted"))j.at("isEnergyDepleted").get_to(info.isEnergyDepleted);
}

void from_json(const nlohmann::json& j, OverHeatInfo& info) {
	if (j.contains("overheatTimer"))j.at("overheatTimer").get_to(info.overheatTimer);
	if (j.contains("overheatDuration"))j.at("overheatDuration").get_to(info.overheatDuration);
	if (j.contains("isOverheated"))j.at("isOverheated").get_to(info.isOverheated);
}

//============================================================================
// 演算子オーバーロード
//============================================================================

//GameCharacterContext GameCharacterContext::operator=(const GameCharacterContext& other) {
//	
//	this->transform = other.transform;
//	this->velocity = other.velocity;
//	this->moveDirection = other.moveDirection;
//	this->focusTargetPos = other.focusTargetPos;
//	this->deceleration = other.deceleration;
//	this->moveSpeed = other.moveSpeed;
//	this->kMaxMoveSpeed = other.kMaxMoveSpeed;
//	this->fallSpeed = other.fallSpeed;
//	this->health = other.health;
//	this->maxHealth = other.maxHealth;
//	this->isAlive = other.isAlive;
//	this->onGround = other.onGround;
//	this->isChargeShooting = other.isChargeShooting;
//	this->isDamaged = other.isDamaged;
//	this->stepBoostInfo = other.stepBoostInfo;
//	this->jumpInfo = other.jumpInfo;
//	this->chargeAttackStunInfo = other.chargeAttackStunInfo;
//	this->energyInfo = other.energyInfo;
//	this->overHeatInfo = other.overHeatInfo;
//	this->name = other.name;
//	return *this;
//}
