#pragma once
#include "application/Provider/IMoveDirectionProvider.h"
#include "application/Entity/Enemy/Enemy.h"

class EnemyDirectionProvider : public IMoveDirectionProvider {

public:
    EnemyDirectionProvider(Enemy* enemy) : enemy_(enemy) {}
    ~EnemyDirectionProvider() override = default;
    // IMoveDirectionProviderの実装
	Vector3 GetMoveDirection() const override;

private:
    Enemy* enemy_; // Enemyオブジェクトへのポインタ
};