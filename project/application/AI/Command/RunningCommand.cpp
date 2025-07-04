#include "RunningCommand.h"
#include "Entity/GameCharacter.h"
#include "math/Vector3Math.h"

RunningCommand::RunningCommand(const Vector3& targetPosition, float moveSpeed)
    : targetPosition_(targetPosition), moveSpeed_(moveSpeed), isComplete_(false) {
}

void RunningCommand::Execute(GameCharacter* character, float deltaTime) {
    if (!character) return;
    
    Vector3 currentPos = character->GetObject3d()->GetTranslate();
    Vector3 direction = Vector3Math::Subtract(targetPosition_, currentPos);
    float distance = Vector3Math::Length(direction);
    
    // 目標位置に到達した判定
    if (distance <= arrivalThreshold_) {
        isComplete_ = true;
        return;
    }
    
    // 方向ベクトルを正規化
    if (distance > 0.0f) {
        direction = Vector3Math::Normalize(direction);
        
        // 移動量を計算
        Vector3 velocity = Vector3Math::Multiply(moveSpeed_ * deltaTime, direction);
        Vector3 newPosition = Vector3Math::Add(currentPos, velocity);
        
        // キャラクターの位置を更新
        character->SetTranslate(newPosition);
    }
}

bool RunningCommand::IsComplete() const {
    return isComplete_;
}

void RunningCommand::Reset() {
    isComplete_ = false;
}

void RunningCommand::SetTargetPosition(const Vector3& targetPosition) {
    targetPosition_ = targetPosition;
    isComplete_ = false;
}

void RunningCommand::SetMoveSpeed(float moveSpeed) {
    moveSpeed_ = moveSpeed;
}