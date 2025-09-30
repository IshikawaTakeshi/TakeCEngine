#pragma once
#include <unordered_map>
#include "application/Entity/GameCharacterInfo.h"
#include "application/Weapon/BaseWeapon.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

using Action = CharacterActionInput;

class AIBrainSystem {
	public:
	AIBrainSystem() = default;
	~AIBrainSystem() = default;

	void Initialize(GameCharacterContext* characterInfo,size_t weaponUnitSize);
	void Update();

	std::vector<int> ChooseWeaponUnit(const std::vector<std::unique_ptr<BaseWeapon>>& weapons);

	Action ChooseBestAction();

	//=============================================================================
	// accessor
	//=============================================================================

	//--- getter ---
	float GetAttackScore(int index) const { return attackScores_[index]; }
	float GetDistanceToTarget() const { return distanceToTarget_; }
	float GetOrbitRadius() const { return orbitRadius_; }
	bool GetIsBulletNearby() const { return isBulletNearby_; }
	Action GetBestAction() const { return bestAction_; }

	//--- setter ---
	void SetDistanceToTarget(float distance) { distanceToTarget_ = distance; }
	void SetOrbitRadius(float radius) { orbitRadius_ = radius; }
	void SetIsBulletNearby(bool isNearby) { isBulletNearby_ = isNearby; }

private:

	Action bestAction_ = Action::NONE;
	std::vector<float> attackScores_;
	std::unordered_map<CharacterActionInput, float> actionScores_;

	float distanceToTarget_;
	float orbitRadius_;

	//敵の弾が近くに飛んできたかどうか
	bool isBulletNearby_ = false;
	//enemyの情報(借りてくる)
	GameCharacterContext* characterInfo_;

private:

	float CalculateHpScore();
	float CalculateAttackScore(BaseWeapon* weapon);
	float CalculateStepBoostScore();
	float CalculateJumpScore();

	
};

