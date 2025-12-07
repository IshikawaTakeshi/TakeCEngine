#pragma once
#include "engine/Input/IInputDevice.h"
#include "engine/Input/Input.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

class PlayerPadInput : public IInputDevice<CharacterActionInput> {
public:
    PlayerPadInput() = default;
    ~PlayerPadInput() override = default;

    float GetVector(CharacterActionInput axis) const override;
    Vector2 GetVector2(CharacterActionInput axis) const override;

    bool IsPressed(CharacterActionInput button) const override;

    bool IsTriggered(CharacterActionInput button) const override;
};

