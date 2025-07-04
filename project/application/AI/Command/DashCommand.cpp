#include "DashCommand.h"
#include "Entity/GameCharacter.h"
#include "math/Vector3Math.h"

DashCommand::DashCommand(const Vector3& direction, float dashSpeed, float duration)
    : direction_(Vector3Math::Normalize(direction)), dashSpeed_(dashSpeed), 
      duration_(duration), currentTime_(0.0f), isComplete_(false) {
}

void DashCommand::Execute(GameCharacter* character, float deltaTime) {
    if (!character || isComplete_) return;
    
    currentTime_ += deltaTime;
    
    // ダッシュ時間が経過した場合
    if (currentTime_ >= duration_) {
        isComplete_ = true;
        return;
    }
    
    // ダッシュ移動の実行
    Vector3 currentPos = character->GetObject3d()->GetTranslate();
    Vector3 velocity = Vector3Math::Multiply(dashSpeed_ * deltaTime, direction_);
    Vector3 newPosition = Vector3Math::Add(currentPos, velocity);
    
    character->SetTranslate(newPosition);
}

bool DashCommand::IsComplete() const {
    return isComplete_;
}

void DashCommand::Reset() {
    currentTime_ = 0.0f;
    isComplete_ = false;
}

void DashCommand::SetDirection(const Vector3& direction) {
    direction_ = Vector3Math::Normalize(direction);
}

void DashCommand::SetDashSpeed(float dashSpeed) {
    dashSpeed_ = dashSpeed;
}

void DashCommand::SetDuration(float duration) {
    duration_ = duration;
}