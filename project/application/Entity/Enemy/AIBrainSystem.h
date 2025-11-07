#pragma once
#include <unordered_map>
#include "application/Entity/GameCharacterInfo.h"
#include "application/Weapon/BaseWeapon.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

// CharacterActionInput -> Actionに名前設定
using Action = CharacterActionInput;

//===================================================================================
// AIBrainSystem class
//===================================================================================
class AIBrainSystem {
	public:
	AIBrainSystem() = default;
	~AIBrainSystem() = default;

	//=============================================================================
	// functions
	//=============================================================================

	//初期化
	void Initialize(PlayableCharacterInfo* characterInfo,size_t weaponUnitSize);
	//更新
	void Update();
	//ImGuiの更新
	void UpdateImGui();
	//武器ユニットの選択
	std::vector<int> ChooseWeaponUnit(const std::vector<std::unique_ptr<BaseWeapon>>& weapons);
	//最適な行動の選択
	Action ChooseBestAction();

	//=============================================================================
	// accessor
	//=============================================================================

	//----- getter -----------------------------------

	//体力スコアの取得
	float GetAttackScore(int index) const { return attackScores_[index]; }
	//行動スコアの取得
	float GetDistanceToTarget() const { return distanceToTarget_; }
	//軌道半径の取得
	float GetOrbitRadius() const { return orbitRadius_; }
	//敵の弾が近くに飛んできたかどうか
	bool GetIsBulletNearby() const { return isBulletNearby_; }
	//最適な行動の取得
	Action GetBestAction() const { return bestAction_; }

	//----- setter -----------------------------------

	//行動スコアの設定
	void SetDistanceToTarget(float distance) { distanceToTarget_ = distance; }
	//軌道半径の設定
	void SetOrbitRadius(float radius) { orbitRadius_ = radius; }
	//敵の弾が近くに飛んできたかどうかの設定
	void SetIsBulletNearby(bool isNearby) { isBulletNearby_ = isNearby; }

private:

	//最適な行動
	Action bestAction_ = Action::NONE;
	//攻撃スコアリスト
	std::vector<float> attackScores_;
	//行動スコアマップ
	std::unordered_map<CharacterActionInput, float> actionScores_;

	//ターゲットまでの距離
	float distanceToTarget_;
	//軌道半径
	float orbitRadius_;

	//敵の弾が近くに飛んできたかどうか
	bool isBulletNearby_ = false;
	//enemyの情報(借りてくる)
	PlayableCharacterInfo* characterInfo_;

private:

	//スコア計算関数群
	float CalculateHpScore();
	float CalculateAttackScore(BaseWeapon* weapon);
	float CalculateStepBoostScore();
	float CalculateJumpScore();
	float CalculateFloatingScore();
	
};