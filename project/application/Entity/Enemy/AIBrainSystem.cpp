#include "AIBrainSystem.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"
#include <cmath>

void AIBrainSystem::Initialize(GameCharacterContext* characterInfo,size_t weaponUnitSize) {
	characterInfo_ = characterInfo;
	attackScores_.resize(weaponUnitSize, 0.0f);
}

void AIBrainSystem::Update() {
	//各スコアの計算
	actionScores_[Action::USE_REAIR] = CalculateHpScore();
	actionScores_[Action::STEPBOOST] = CalculateStepBoostScore();
	actionScores_[Action::JUMP] = CalculateJumpScore();

	actionScores_[Action::ATTACK_RA] = attackScores_[0]; // 右手武器のスコア
	actionScores_[Action::ATTACK_LA] = attackScores_[1]; // 左手武器のスコア
	//actionScores_[Action::ATTACK_RB] = attackScores_[2]; // 右肩武器のスコア
	//actionScores_[Action::ATTACK_LB] = attackScores_[3]; // 左肩武器のスコア

	bestAction_ = ChooseBestAction();
}

void AIBrainSystem::UpdateImGui() {
	ImGui::SeparatorText("AIBrainSystem");
	ImGui::Text("DistanceToTarget: %.2f", distanceToTarget_);
	ImGui::Text("OrbitRadius: %.2f", orbitRadius_);
	ImGui::Text("IsBulletNearby: %s", isBulletNearby_ ? "True" : "False");
	ImGui::Separator();
	ImGui::Text("BestAction: %s", StringUtility::EnumToString(bestAction_).c_str());
	ImGui::Separator();
	for (const auto& [action, score] : actionScores_) {
		std::string actionName = "Score " + StringUtility::EnumToString<CharacterActionInput>(action);
		ImGui::SliderFloat(actionName.c_str(), &actionScores_[action], 0.0f, 1.0f);
	}
}

float AIBrainSystem::CalculateHpScore() {
	float hpRatio = characterInfo_->health / characterInfo_->maxHealth;
	return 1.0f - hpRatio; // HPが低いほどスコアが高くなる
}

float AIBrainSystem::CalculateAttackScore(BaseWeapon* weapon) {
	float attackScore;

	float distanceFactor = 1.0f - (distanceToTarget_ / weapon->GetEffectiveRange());
	float cooldownFactor = weapon->GetIsAvailable() ? 1.0f : 0.0f;
	
	attackScore = distanceFactor * 0.8f * cooldownFactor;
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

Action AIBrainSystem::ChooseBestAction() {
	Action bestAction = Action::NONE;

	//各スコアから最も高いスコアの行動を選択
	std::vector<float> chooseScores = {
		actionScores_[Action::USE_REAIR],
		actionScores_[Action::STEPBOOST],
		actionScores_[Action::JUMP],
	};

	float bestScore = 0.0f;
	for (const auto& [action, score] : actionScores_) {
		if (score > bestScore) {
			bestScore = score;
			bestAction = action;
		}
	}

	return bestAction;
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
	float hpFactor = 1.0f - (characterInfo_->health / characterInfo_->maxHealth);

	stepBoostScore = dangerFactor * energyFactor * (hpFactor * 1.3f);
	stepBoostScore = std::clamp(stepBoostScore, 0.0f, 1.0f); // 0から1の範囲にクランプ

	return stepBoostScore;
}

float AIBrainSystem::CalculateJumpScore() {
	
	float jumpScore = 0.0f;
	if (characterInfo_->onGround == true && characterInfo_->overHeatInfo.isOverheated == false) {

		//相手との距離(特にY軸方向)が一定以上離れている場合はジャンプを優先する
		if (std::abs(characterInfo_->transform.translate.y - distanceToTarget_) > orbitRadius_ * 0.5f) {
			jumpScore += 0.5f; // 基本スコア
		}

		// エネルギー残量に基づくスコア計算
		float energyFactor = (characterInfo_->energyInfo.energy - characterInfo_->jumpInfo.useEnergy) / characterInfo_->energyInfo.maxEnergy;
		energyFactor = std::clamp(energyFactor, 0.0f, 1.0f);
		// 近接しているかどうかに基づくスコア計算
		float dangerFactor = isBulletNearby_ ? 1.0f : 0.0f;

		jumpScore += dangerFactor * energyFactor * 0.5f; // 最大で0.5加算
	}

	return jumpScore;
}
