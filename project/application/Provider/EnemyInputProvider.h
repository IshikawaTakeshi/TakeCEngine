#pragma once
#include "application/Provider/BaseInputProvider.h"
#include "application/Entity/Enemy/AIBrainSystem.h"

class Enemy;
class EnemyInputProvider : public baseInputProvider {

public:
    EnemyInputProvider(Enemy* enemy) : enemy_(enemy) {}
    ~EnemyInputProvider() override = default;
    // IMoveDirectionProviderの実装
	Vector3 GetMoveDirection() const override;
	bool IsJumpRequested() const override;
	void RequestAttack() override;
	void RequestChargeAttack() override;

	void SetAIBrainSystem(AIBrainSystem* aiBrainSystem) { aiBrainSystem_ = aiBrainSystem; }

private:

	AIBrainSystem* aiBrainSystem_; // AIBrainSystemオブジェクトへのポインタ
    Enemy* enemy_; // Enemyオブジェクトへのポインタ
};