#pragma once
#include "ICommand.h"
#include "math/Vector3.h"

/// <summary>
/// ステップブーストコマンド（素早い回避移動）
/// </summary>
class StepBoostCommand : public ICommand {
public:
    StepBoostCommand(const Vector3& direction, float stepSpeed = 200.0f, float duration = 0.3f);
    ~StepBoostCommand() override = default;
    
    void Execute(GameCharacter* character, float deltaTime) override;
    bool IsComplete() const override;
    void Reset() override;
    
    void SetDirection(const Vector3& direction);
    void SetStepSpeed(float stepSpeed);
    void SetDuration(float duration);

private:
    Vector3 direction_;
    float stepSpeed_;
    float duration_;
    float currentTime_;
    bool isComplete_;
};