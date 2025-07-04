#include "IdleState.h"
#include "AI/Command/IdleCommand.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "math/Vector3Math.h"

void IdleState::Enter(Enemy* enemy) {
    if (!enemy) return;
    
    // 待機コマンドを設定
    currentCommand_ = std::make_unique<IdleCommand>();
}

std::string IdleState::Update(Enemy* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return "";
    
    // プレイヤーとの距離をチェック
    Vector3 enemyPos = enemy->GetObject3d()->GetTranslate();
    Vector3 playerPos = player->GetObject3d()->GetTranslate();
    Vector3 toPlayer = Vector3Math::Subtract(playerPos, enemyPos);
    float distance = Vector3Math::Length(toPlayer);
    
    // プレイヤーが検出範囲内にいる場合、追跡状態に遷移
    if (distance <= detectionRange_) {
        return "Chase";
    }
    
    // 待機コマンドを実行
    if (currentCommand_) {
        currentCommand_->Execute(enemy, deltaTime);
    }
    
    return ""; // 状態変更なし
}

void IdleState::Exit(Enemy* enemy) {
    currentCommand_.reset();
}