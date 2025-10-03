#pragma once
#include "application/Provider/BaseInputProvider.h"

class Enemy;
class EnemyInputProvider : public baseInputProvider {

public:
    EnemyInputProvider(Enemy* enemy) : enemy_(enemy) {}
    ~EnemyInputProvider() override = default;
    // IMoveDirectionProviderの実装
	Vector3 GetMoveDirection() const override;
	void RequestAttack() override;
	void RequestChargeAttack() override;

private:
    Enemy* enemy_; // Enemyオブジェクトへのポインタ
};