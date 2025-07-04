#pragma once
#include "ICommand.h"
#include "math/Vector3.h"

/// <summary>
/// 移動コマンド
/// </summary>
class RunningCommand : public ICommand {
public:
    RunningCommand(const Vector3& targetPosition, float moveSpeed = 50.0f);
    ~RunningCommand() override = default;
    
    void Execute(GameCharacter* character, float deltaTime) override;
    bool IsComplete() const override;
    void Reset() override;
    
    void SetTargetPosition(const Vector3& targetPosition);
    void SetMoveSpeed(float moveSpeed);

private:
    Vector3 targetPosition_;
    float moveSpeed_;
    bool isComplete_;
    const float arrivalThreshold_ = 2.0f; // 到達判定の閾値
};