#pragma once
#include "ICommand.h"
#include "math/Vector3.h"

/// <summary>
/// ダッシュコマンド
/// </summary>
class DashCommand : public ICommand {
public:
    DashCommand(const Vector3& direction, float dashSpeed = 200.0f, float duration = 0.5f);
    ~DashCommand() override = default;
    
    void Execute(GameCharacter* character, float deltaTime) override;
    bool IsComplete() const override;
    void Reset() override;
    
    void SetDirection(const Vector3& direction);
    void SetDashSpeed(float dashSpeed);
    void SetDuration(float duration);

private:
    Vector3 direction_;
    float dashSpeed_;
    float duration_;
    float currentTime_;
    bool isComplete_;
};