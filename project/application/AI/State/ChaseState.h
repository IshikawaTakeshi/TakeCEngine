#pragma once
#include "AIState.h"

/// <summary>
/// 追跡状態
/// プレイヤーを追いかけ、攻撃範囲内に入ったら攻撃状態に遷移
/// </summary>
class ChaseState : public AIState {
public:
    ChaseState() = default;
    ~ChaseState() override = default;
    
    void Enter(Enemy* enemy) override;
    std::string Update(Enemy* enemy, Player* player, float deltaTime) override;
    void Exit(Enemy* enemy) override;

private:
    const float attackRange_ = 8.0f;    // 攻撃範囲
    const float loseRange_ = 50.0f;     // 見失う距離
    const float dashThreshold_ = 15.0f; // ダッシュを使用する距離
};