#pragma once
#include "ICommand.h"

class BaseWeapon;

/// <summary>
/// 攻撃コマンド
/// </summary>
class AttackCommand : public ICommand {
public:
    AttackCommand();
    ~AttackCommand() override = default;
    
    void Execute(GameCharacter* character, float deltaTime) override;
    bool IsComplete() const override;
    void Reset() override;

private:
    bool isComplete_;
    bool hasAttacked_;
};