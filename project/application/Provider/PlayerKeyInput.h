#pragma once
#include "engine/Input/IInputDevice.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

//============================================================================
// PlayerKeyInput class
//============================================================================
class PlayerKeyInput : public IInputDevice<CharacterActionInput> {
public:

	PlayerKeyInput() = default;
	~PlayerKeyInput() override = default;

	//--------- accessor -----------------------------------------------------
	// 連続入力
	float GetVector(CharacterActionInput axis)  const override;
	Vector2 GetVector2(CharacterActionInput axis) const override;
	// 単入力
	bool IsPressed(CharacterActionInput button) const override;
	bool IsTriggered(CharacterActionInput button) const override;
	bool IsReleased(CharacterActionInput button) const override;

private:

	float mouseMoveScale_ = 2.5f;
};