#pragma once
#include "AIState.h"
#include "math/Vector3.h"
#include <vector>

/// <summary>
/// 巡回状態
/// 決められたルートを巡回し、プレイヤーを発見したら追跡状態に遷移
/// </summary>
class PatrolState : public AIState {
public:
    PatrolState(const std::vector<Vector3>& patrolPoints);
    ~PatrolState() override = default;
    
    void Enter(Enemy* enemy) override;
    std::string Update(Enemy* enemy, Player* player, float deltaTime) override;
    void Exit(Enemy* enemy) override;
    
    void SetPatrolPoints(const std::vector<Vector3>& patrolPoints);

private:
    std::vector<Vector3> patrolPoints_;
    int currentPointIndex_;
    const float detectionRange_ = 25.0f; // プレイヤー発見距離
};