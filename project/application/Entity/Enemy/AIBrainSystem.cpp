#include "AIBrainSystem.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"
#include "engine/math/Easing.h"
#include "engine/Collision/CollisionManager.h"
#include "engine/MAth/Vector3Math.h"
#include <cmath>

//===================================================================================
//　初期化
//===================================================================================
void AIBrainSystem::Initialize(PlayableCharacterInfo* characterInfo,size_t weaponUnitSize) {
	characterInfo_ = characterInfo;
	attackScores_.resize(weaponUnitSize, 0.0f);
}

//===================================================================================
//　更新
//===================================================================================
void AIBrainSystem::Update() {
	//各スコアの計算
	actionScores_[Action::USE_REAIR] = CalculateHpScore();
	actionScores_[Action::STEPBOOST] = CalculateStepBoostScore();
	actionScores_[Action::JUMP] = CalculateJumpScore();
	actionScores_[Action::FLOATING] = CalculateFloatingScore();

	actionScores_[Action::ATTACK_RA] = attackScores_[0]; // 右手武器のスコア
	actionScores_[Action::ATTACK_LA] = attackScores_[1]; // 左手武器のスコア

	//TODO: Enemyの肩武器の設定
	actionScores_[Action::ATTACK_RB] = attackScores_[2]; // 右肩武器のスコア
	actionScores_[Action::ATTACK_LB] = attackScores_[3]; // 左肩武器のスコア

	bestAction_ = ChooseBestAction();
}

//===================================================================================
//　ImGui更新
//===================================================================================
void AIBrainSystem::UpdateImGui() {
	ImGui::SeparatorText("AIBrainSystem");
	ImGui::Text("DistanceToTarget: %.2f", distanceToTarget_);
	ImGui::Text("OrbitRadius: %.2f", orbitRadius_);
	ImGui::Text("IsBulletNearby: %s", isBulletNearby_ ? "True" : "False");
	ImGui::Separator();
	ImGui::Text("BestAction: %s", StringUtility::EnumToString(bestAction_).c_str());
	ImGui::Separator();

	//各スコアの表示と調整
	for (const auto& [action, score] : actionScores_) {
		std::string actionName = "Score " + StringUtility::EnumToString<CharacterActionInput>(action);
		ImGui::SliderFloat(actionName.c_str(), &actionScores_[action], 0.0f, 1.0f);
	}
}

//===================================================================================
//　スコア計算(HP)
//===================================================================================
float AIBrainSystem::CalculateHpScore() {
	float hpRatio = characterInfo_->health / characterInfo_->maxHealth;
	float normalizedHpLoss = 1.0f - hpRatio;

	// HP低下が深刻になるほど急激にスコアが上昇
	// InExpoを使うことで、HPが30%以下になると急激に回復の優先度が上がる
	return Easing::EaseInExpo(normalizedHpLoss);
}

//===================================================================================
//　スコア計算(攻撃)
//===================================================================================
float AIBrainSystem::CalculateAttackScore(BaseWeapon* weapon) {
	if (!weapon->GetIsAvailable()) {
		return 0.0f; // クールダウン中は攻撃不可
	}

	float effectiveRange = weapon->GetEffectiveRange();
	float distanceRatio = std::clamp(distanceToTarget_ / effectiveRange, 0.0f, 1.0f);

	// 有効射程内では高スコア、射程外では急激に低下
	// OutCubicで自然な減衰カーブを作成
	float distanceFactor = 1.0f - Easing::EaseOutCubic(distanceRatio);

	float attackScore = distanceFactor * 0.8f;
	return std::clamp(attackScore, 0.0f, 1.0f);
}

//===================================================================================
//　武器ユニット選択
//===================================================================================
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

//===================================================================================
//　一番良い行動選択
//===================================================================================
Action AIBrainSystem::ChooseBestAction() {
	Action bestAction = Action::NONE;

	//各スコアから最も高いスコアの行動を選択
	std::vector<float> chooseScores = {
		actionScores_[Action::USE_REAIR],
		actionScores_[Action::STEPBOOST],
		actionScores_[Action::JUMP],
		actionScores_[Action::FLOATING],
	};


	// 各行動のスコアを比較して最も高いものを選択
	float bestScore = 0.0f;
	for (const auto& [action, score] : actionScores_) {
		if (score > bestScore) {
			bestScore = score;
			bestAction = action;
		}
	}

	return bestAction;
}

//===================================================================================
//　スコア計算(ステップブースト)
//===================================================================================
float AIBrainSystem::CalculateStepBoostScore() {
	if (characterInfo_->overHeatInfo.isOverheated) {
		return 0.0f;
	}

	// エネルギー残量の計算
	float energyAfterBoost = characterInfo_->energyInfo.energy - characterInfo_->stepBoostInfo.useEnergy;
	if (energyAfterBoost < 0.0f) {
		return 0.0f; // エネルギー不足
	}

	float energyRatio = energyAfterBoost / characterInfo_->energyInfo.maxEnergy;
	// エネルギーが十分にある時は高スコア、少ない時は慎重に
	float energyFactor = Easing::EaseOutQuad(energyRatio);

	// 危険度の計算
	float dangerFactor = isBulletNearby_ ? 0.7f : 0.05f;

	// HPが低いほど回避行動を優先
	float hpRatio = characterInfo_->health / characterInfo_->maxHealth;
	float hpFactor = 1.0f - hpRatio;
	// HPが低い時に急激に回避を優先
	float urgencyFactor = Easing::EaseInQuad(hpFactor) * 1.5f;

	// 最終スコア計算
	float stepBoostScore = dangerFactor * energyFactor * (1.0f + urgencyFactor);
	return std::clamp(stepBoostScore, 0.0f, 1.0f);
}

//===================================================================================
//　スコア計算(ジャンプ)
//===================================================================================
float AIBrainSystem::CalculateJumpScore() {
	
	float jumpScore = 0.0f;
	// 地面にいない、またはオーバーヒート中はジャンプ不可
	if (!characterInfo_->onGround || characterInfo_->overHeatInfo.isOverheated) {
		return 0.0f;
	}

	// エネルギーチェック
	float energyAfterJump = characterInfo_->energyInfo.energy - characterInfo_->jumpInfo.useEnergy;
	if (energyAfterJump < 0.0f) {
		return 0.0f;
	}

	// 高度差による計算
	float heightDifference = std::abs(distanceToTarget_ - characterInfo_->transform.translate.y);
	float heightThreshold = orbitRadius_ * 0.7f;

	if (heightDifference > heightThreshold) {
		float heightRatio = std::clamp(heightDifference / (heightThreshold * 2.0f), 0.0f, 1.0f);
		// 高度差が大きいほど滑らかにスコア上昇
		jumpScore += Easing::EaseInOutSine(heightRatio) * 0.3f;
	}

	// エネルギー要因
	float energyRatio = energyAfterJump / characterInfo_->energyInfo.maxEnergy;
	float energyFactor = Easing::GentleRise(energyRatio);
	// エネルギーが多いほどスコア上昇
	jumpScore += energyFactor * 0.15f;

	return std::clamp(jumpScore, 0.0f, 1.0f);
}

//===================================================================================
//　スコア計算(フローティング)
//===================================================================================
float AIBrainSystem::CalculateFloatingScore() {
	
	float floatingScore = 0.0f;

	// オーバーヒート中は上昇できない
	if (characterInfo_->overHeatInfo.isOverheated) {
		return 0.0f;
	}

	// ターゲットへの方向と距離を計算
	Vector3 targetDirection = Vector3Math::Normalize(characterInfo_->focusTargetPos - characterInfo_->transform.translate);
	float distance = Vector3Math::Length(characterInfo_->focusTargetPos - characterInfo_->transform.translate);

	// レイキャストで障害物の有無を確認
	Ray ray;
	ray.origin = characterInfo_->transform.translate;
	ray.direction = targetDirection;
	ray.distance = distance;
	RayCastHit hitInfo;
	float obstacleFactor = 0.2f;

	// 目標: プレイヤーより少し上（例: +2.0f）
	float targetOffset = 2.0f;
	if (distanceToTarget_ < 20.0f) { // 近距離なら
		targetOffset = 10.0f; // 相手のカメラアングルを狂わせる高さまで飛ぶ
	}

	float idealHeight = characterInfo_->focusTargetPos.y + targetOffset;
	float currentHeight = characterInfo_->transform.translate.y;
	float heightDiff = idealHeight - currentHeight;

	//コライダーのマスク
	uint32_t layerMask = ~static_cast<uint32_t>(CollisionLayer::Ignoe);
	if (CollisionManager::GetInstance().RayCast(ray, hitInfo,layerMask) == true) {

		// プレイヤーとのY軸方向の位置差を計算
		float verticalDiff = characterInfo_->focusTargetPos.y - characterInfo_->transform.translate.y;

		// プレイヤーより下にいる、または同じ高さにいる場合
		if (verticalDiff >= 0.0f) {
			
			// 障害物との距離が近いほどスコアを高くする
			float obstacleProximity = 1.0f - std::clamp(hitInfo.distance / distance, 0.0f, 1.0f);
			obstacleFactor = Easing::UrgentRise(obstacleProximity);
		}
	}

	// 高度差を0-1の範囲に正規化（最大10.0fの差を想定）
	const float maxHeightDiff = 4.0f;
	float normalizedHeightDiff = std::clamp(heightDiff / maxHeightDiff, 0.0f, 1.0f);

	// Easing関数を使用して緩やかなスコア変化
	float heightFactor = Easing::GentleRise(normalizedHeightDiff);

	// エネルギー残量に基づくスコア計算 (エネルギーが多いほど高スコア)
	float energyFactor = (characterInfo_->energyInfo.energy - characterInfo_->jumpInfo.useEnergy) / characterInfo_->energyInfo.maxEnergy;
	energyFactor = std::clamp(energyFactor, 0.0f, 1.0f);

	// 最終スコア計算（障害物要素を追加）
	floatingScore = heightFactor * energyFactor * obstacleFactor * 0.9f;
	floatingScore = Easing::Lerp(0.0f, 1.0f, floatingScore);

	return floatingScore;
}