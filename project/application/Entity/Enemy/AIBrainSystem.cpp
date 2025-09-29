#include "AIBrainSystem.h"
#include <cmath>

void AIBrainSystem::Initialize(GameCharacterContext* characterInfo,size_t weaponUnitSize) {
	characterInfo_ = characterInfo;
	hpScore_ = 0.0f;
	distanceScore_ = 0.0f;
	attackScores_.resize(weaponUnitSize, 0.0f);
	stepBoostScore_ = 0.0f;
}

void AIBrainSystem::Update() {
	//各スコアの計算
	hpScore_ = CalculateHpScore();
	distanceScore_ = CalculateDistanceScore();
	stepBoostScore_ = CalculateStepBoostScore();
}

float AIBrainSystem::CalculateHpScore() {
	float hpRatio = characterInfo_->health / characterInfo_->maxHealth;
	return 1.0f - hpRatio; // HPが低いほどスコアが高くなる
}

float AIBrainSystem::CalculateDistanceScore() {
	float distanceScore = distanceToTarget_ / orbitRadius_;
	return std::clamp(distanceScore, 0.0f, 1.0f); // 0から1の範囲にクランプ
}

float AIBrainSystem::CalculateAttackScore(BaseWeapon* weapon) {
	float attackScore;

	float distanceFactor = 1.0f - (distanceToTarget_ / weapon->GetEffectiveRange());
	float cooldownFactor = weapon->GetIsAvailable() ? 1.0f : 0.0f;
	
	attackScore = distanceFactor * cooldownFactor;
	return std::clamp(attackScore, 0.0f, 1.0f); // 0から1の範囲にクランプ
}

std::vector<int> AIBrainSystem::ChooseWeaponUnit(const std::vector<std::unique_ptr<BaseWeapon>>& weapons) {
	std::vector<int> chosenWeapons;

	for(int i = 0; i < weapons.size(); ++i) {
		attackScores_[i] = CalculateAttackScore(weapons[i].get());
		if(attackScores_[i] > 0.5f) { // スコアが0.5以上の武器を選択
			chosenWeapons.push_back(i);
		}
	}

	return chosenWeapons;
}

float AIBrainSystem::CalculateStepBoostScore() {

	float stepBoostScore;
	if(characterInfo_->overHeatInfo.isOverheated) {
		stepBoostScore = 0.0f; // オーバーヒート中はスコア0
		return stepBoostScore;
	}

	// エネルギー残量に基づくスコア計算
	float energyFactor = (characterInfo_->energyInfo.energy - characterInfo_->stepBoostInfo.useEnergy) / characterInfo_->energyInfo.maxEnergy;
	energyFactor = std::clamp(energyFactor, 0.0f, 1.0f);
	// 近接しているかどうかに基づくスコア計算
	float dangerFactor = isBulletNearby_ ? 1.0f : 0.0f;
	

	stepBoostScore = dangerFactor * energyFactor;

	return stepBoostScore;
}