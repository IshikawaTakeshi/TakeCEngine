#include "PatrolState.h"
#include "AI/Command/RunningCommand.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "math/Vector3Math.h"

PatrolState::PatrolState(const std::vector<Vector3>& patrolPoints)
    : patrolPoints_(patrolPoints), currentPointIndex_(0) {
}

void PatrolState::Enter(Enemy* enemy) {
    if (!enemy || patrolPoints_.empty()) return;
    
    // 最初の巡回ポイントに向かう移動コマンドを設定
    currentCommand_ = std::make_unique<RunningCommand>(patrolPoints_[currentPointIndex_], 40.0f);
}

std::string PatrolState::Update(Enemy* enemy, Player* player, float deltaTime) {
    if (!enemy || !player || patrolPoints_.empty()) return "";
    
    // プレイヤーとの距離をチェック
    Vector3 enemyPos = enemy->GetObject3d()->GetTranslate();
    Vector3 playerPos = player->GetObject3d()->GetTranslate();
    Vector3 toPlayer = Vector3Math::Subtract(playerPos, enemyPos);
    float distance = Vector3Math::Length(toPlayer);
    
    // プレイヤーが検出範囲内にいる場合、追跡状態に遷移
    if (distance <= detectionRange_) {
        return "Chase";
    }
    
    // 現在の巡回ポイントに到達した場合、次のポイントを設定
    if (currentCommand_ && currentCommand_->IsComplete()) {
        currentPointIndex_ = (currentPointIndex_ + 1) % patrolPoints_.size();
        currentCommand_ = std::make_unique<RunningCommand>(patrolPoints_[currentPointIndex_], 40.0f);
    }
    
    // 移動コマンドを実行
    if (currentCommand_) {
        currentCommand_->Execute(enemy, deltaTime);
    }
    
    return ""; // 状態変更なし
}

void PatrolState::Exit(Enemy* enemy) {
    currentCommand_.reset();
}

void PatrolState::SetPatrolPoints(const std::vector<Vector3>& patrolPoints) {
    patrolPoints_ = patrolPoints;
    currentPointIndex_ = 0;
}