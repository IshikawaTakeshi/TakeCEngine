#pragma once
#include "AIState.h"

/// <summary>
/// 待機状態
/// プレイヤーを探索し、発見したら追跡状態に遷移
/// </summary>
class IdleState : public AIState {
public:
    IdleState() = default;
    ~IdleState() override = default;
    
    void Enter(Enemy* enemy) override;
    std::string Update(Enemy* enemy, Player* player, float deltaTime) override;
    void Exit(Enemy* enemy) override;

private:
    const float detectionRange_ = 30.0f; // プレイヤー発見距離
};