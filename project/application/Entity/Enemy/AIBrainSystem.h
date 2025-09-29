#pragma once
#include "application/Entity/GameCharacterInfo.h"
#include "application/Weapon/BaseWeapon.h"

class AIBrainSystem {
	public:
	AIBrainSystem() = default;
	~AIBrainSystem() = default;

	void Initialize(GameCharacterContext* characterInfo,size_t weaponUnitSize);
	void Update();

	

	//=============================================================================
	// accessor
	//=============================================================================

	//--- getter ---
	float GetHpScore() const { return hpScore_; }
	float GetDistanceScore() const { return distanceScore_; }
	float GetAttackScore(int index) const { return attackScores_[index]; }
	float GetStepBoostScore() const { return stepBoostScore_; }
	float GetDistanceToTarget() const { return distanceToTarget_; }
	float GetOrbitRadius() const { return orbitRadius_; }
	bool GetIsBulletNearby() const { return isBulletNearby_; }

	//--- setter ---
	void SetDistanceToTarget(float distance) { distanceToTarget_ = distance; }
	void SetOrbitRadius(float radius) { orbitRadius_ = radius; }
	void SetIsBulletNearby(bool isNearby) { isBulletNearby_ = isNearby; }

private:

	float hpScore_;
	float distanceScore_;
	std::vector<float> attackScores_;
	float stepBoostScore_;

	float distanceToTarget_;
	float orbitRadius_;

	//敵の弾が近くに飛んできたかどうか
	bool isBulletNearby_ = false;

	//enemyの情報(借りてくる)
	GameCharacterContext* characterInfo_;

private:

	float CalculateHpScore();
	float CalculateDistanceScore();
	float CalculateAttackScore(BaseWeapon* weapon);
	float CalculateStepBoostScore();

	std::vector<int> ChooseWeaponUnit(const std::vector<std::unique_ptr<BaseWeapon>>& weapons);
};

