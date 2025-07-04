#include "ChaseState.h"
#include "AI/Command/RunningCommand.h"
#include "AI/Command/DashCommand.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "math/Vector3Math.h"

void ChaseState::Enter(Enemy* enemy) {
    if (!enemy) return;
    
    // 追跡開始時は移動コマンドは後で設定
    currentCommand_.reset();
}

std::string ChaseState::Update(Enemy* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return "";
    
    Vector3 enemyPos = enemy->GetObject3d()->GetTranslate();
    Vector3 playerPos = player->GetObject3d()->GetTranslate();
    Vector3 toPlayer = Vector3Math::Subtract(playerPos, enemyPos);
    float distance = Vector3Math::Length(toPlayer);
    
    // プレイヤーを見失った場合
    if (distance > loseRange_) {
        return "Idle";
    }
    
    // 攻撃範囲内に入った場合
    if (distance <= attackRange_) {
        return "Attack";
    }
    
    // 現在のコマンドが完了しているか、新しいコマンドが必要な場合
    bool needNewCommand = !currentCommand_ || currentCommand_->IsComplete();
    
    if (needNewCommand) {
        // 距離に応じてダッシュか通常移動かを決定
        if (distance > dashThreshold_) {
            Vector3 direction = Vector3Math::Normalize(toPlayer);
            currentCommand_ = std::make_unique<DashCommand>(direction, 150.0f, 0.5f);
        } else {
            currentCommand_ = std::make_unique<RunningCommand>(playerPos, 80.0f);
        }
    }
    
    // コマンドを実行
    if (currentCommand_) {
        currentCommand_->Execute(enemy, deltaTime);
    }
    
    return ""; // 状態変更なし
}

void ChaseState::Exit(Enemy* enemy) {
    currentCommand_.reset();
}