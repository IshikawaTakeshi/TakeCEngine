#include "StepBoostCommand.h"
#include "Entity/GameCharacter.h"
#include "math/Vector3Math.h"

StepBoostCommand::StepBoostCommand(const Vector3& direction, float stepSpeed, float duration)
    : direction_(Vector3Math::Normalize(direction)), stepSpeed_(stepSpeed), 
      duration_(duration), currentTime_(0.0f), isComplete_(false) {
}

void StepBoostCommand::Execute(GameCharacter* character, float deltaTime) {
    if (!character || isComplete_) return;
    
    currentTime_ += deltaTime;
    
    // ステップブースト時間が経過した場合
    if (currentTime_ >= duration_) {
        isComplete_ = true;
        return;
    }
    
    // ステップブースト移動の実行
    Vector3 currentPos = character->GetObject3d()->GetTranslate();
    Vector3 velocity = Vector3Math::Multiply(stepSpeed_ * deltaTime, direction_);
    Vector3 newPosition = Vector3Math::Add(currentPos, velocity);
    
    character->SetTranslate(newPosition);
}

bool StepBoostCommand::IsComplete() const {
    return isComplete_;
}

void StepBoostCommand::Reset() {
    currentTime_ = 0.0f;
    isComplete_ = false;
}

void StepBoostCommand::SetDirection(const Vector3& direction) {
    direction_ = Vector3Math::Normalize(direction);
}

void StepBoostCommand::SetStepSpeed(float stepSpeed) {
    stepSpeed_ = stepSpeed;
}

void StepBoostCommand::SetDuration(float duration) {
    duration_ = duration;
}