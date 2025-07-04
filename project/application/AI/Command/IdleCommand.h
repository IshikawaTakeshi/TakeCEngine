#pragma once
#include "ICommand.h"

/// <summary>
/// 待機コマンド
/// </summary>
class IdleCommand : public ICommand {
public:
    IdleCommand() = default;
    ~IdleCommand() override = default;
    
    void Execute(GameCharacter* character, float deltaTime) override;
    bool IsComplete() const override;
    void Reset() override;

private:
    bool isComplete_ = false;
};