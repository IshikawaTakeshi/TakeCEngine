#pragma once
#include "AIState.h"

/// <summary>
/// 攻撃状態
/// プレイヤーを攻撃し、距離が離れたら追跡状態に戻る
/// </summary>
class AttackState : public AIState {
public:
    AttackState() = default;
    ~AttackState() override = default;
    
    void Enter(Enemy* enemy) override;
    std::string Update(Enemy* enemy, Player* player, float deltaTime) override;
    void Exit(Enemy* enemy) override;

private:
    const float attackRange_ = 8.0f;
    const float cooldownTime_ = 1.0f; // 攻撃後のクールダウン時間
    float currentCooldown_ = 0.0f;
};