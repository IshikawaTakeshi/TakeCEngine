#pragma once
#include "application/Provider/BaseInputProvider.h"
#include "application/Entity/Enemy/AIBrainSystem.h"

// 前方宣言
class Enemy;

//============================================================================
// EnemyInputProvider class
//============================================================================
class EnemyInputProvider : public baseInputProvider {
public:

	EnemyInputProvider(Enemy* enemy);
    ~EnemyInputProvider() override = default;

	//移動方向の取得
	Vector3 GetMoveDirection() const override;
	//ジャンプ入力の取得
	bool RequestJumpInput() const override;
	//攻撃入力の取得
	bool RequestAttack(CharacterActionInput attackButton) override;
	//チャージ攻撃入力の取得
	bool RequestChargeAttack(CharacterActionInput attackButton) override;
	//ステップブースト入力の取得
	bool RequestStepBoost() const override;

	//----- setter ---------------------------
	void SetAIBrainSystem(AIBrainSystem* aiBrainSystem) { aiBrainSystem_ = aiBrainSystem; }

private:

	// AIBrainSystemオブジェクトへのポインタ
	AIBrainSystem* aiBrainSystem_; 
	// Enemyオブジェクトへのポインタ
    Enemy* enemy_; 
};