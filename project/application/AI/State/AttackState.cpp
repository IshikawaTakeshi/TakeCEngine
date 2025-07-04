#include "AttackState.h"
#include "AI/Command/AttackCommand.h"
#include "AI/Command/IdleCommand.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "math/Vector3Math.h"

void AttackState::Enter(Enemy* enemy) {
    if (!enemy) return;
    
    // 攻撃コマンドを設定
    currentCommand_ = std::make_unique<AttackCommand>();
    currentCooldown_ = 0.0f;
}

std::string AttackState::Update(Enemy* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return "";
    
    Vector3 enemyPos = enemy->GetObject3d()->GetTranslate();
    Vector3 playerPos = player->GetObject3d()->GetTranslate();
    Vector3 toPlayer = Vector3Math::Subtract(playerPos, enemyPos);
    float distance = Vector3Math::Length(toPlayer);
    
    // プレイヤーが攻撃範囲外に出た場合
    if (distance > attackRange_) {
        return "Chase";
    }
    
    // クールダウン中の場合
    if (currentCooldown_ > 0.0f) {
        currentCooldown_ -= deltaTime;
        
        // 待機コマンドに切り替え
        if (!currentCommand_ || !dynamic_cast<IdleCommand*>(currentCommand_.get())) {
            currentCommand_ = std::make_unique<IdleCommand>();
        }
    } else {
        // 攻撃可能な場合、攻撃コマンドを実行
        if (!currentCommand_ || !dynamic_cast<AttackCommand*>(currentCommand_.get())) {
            currentCommand_ = std::make_unique<AttackCommand>();
        }
        
        // 攻撃完了後にクールダウンを設定
        if (currentCommand_->IsComplete()) {
            currentCooldown_ = cooldownTime_;
        }
    }
    
    // コマンドを実行
    if (currentCommand_) {
        currentCommand_->Execute(enemy, deltaTime);
    }
    
    return ""; // 状態変更なし
}

void AttackState::Exit(Enemy* enemy) {
    currentCommand_.reset();
    currentCooldown_ = 0.0f;
}