#include "PlayerPadInput.h"

//=============================================================================
// スティック入力取得(float)
//=============================================================================

float PlayerPadInput::GetVector(CharacterActionInput axis) const {
	switch (axis) {
	case CharacterActionInput::MOVE: // 片軸で使う場合
		return Input::GetInstance()->GetLeftStickState(0).y;
	default:
		return 0.0f;
	}
}

//=============================================================================
// スティック入力取得(Vector2)
//=============================================================================
Vector2 PlayerPadInput::GetVector2(CharacterActionInput axis) const {
	Vector2 result = {};
	switch (axis) {
	case CharacterActionInput::MOVE:
		result = Input::GetInstance()->GetLeftStickState(0);

		// 正規化
		if (result.Length() > 1.0f) {
			result = result.Normalize();
		}
		return result;
	case CharacterActionInput::MOVE_CAMERA:
		result = Input::GetInstance()->GetRightStickState(0);

		// 正規化
		if (result.Length() > 1.0f) {
			result = result.Normalize();
		}
		return result;
	default:
		return { 0.0f, 0.0f };
	}
}

//=============================================================================
// ボタン入力取得
//=============================================================================
bool PlayerPadInput::IsPressed(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->PushButton(0, GamepadButtonType::RT);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->PushButton(0, GamepadButtonType::A);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->PushButton(0, GamepadButtonType::LT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->PushButton(0, GamepadButtonType::RB);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->PushButton(0, GamepadButtonType::LB);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->PushButton(0, GamepadButtonType::X);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->PushButton(0, GamepadButtonType::Y);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->PushButton(0, GamepadButtonType::RightStick);
	default:
		return false;
	}
}

//=============================================================================
// ボタン入力取得(トリガー)
//=============================================================================
bool PlayerPadInput::IsTriggered(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->TriggerButton(0, GamepadButtonType::RT);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->TriggerButton(0, GamepadButtonType::A);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->TriggerButton(0, GamepadButtonType::LT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->TriggerButton(0, GamepadButtonType::RB);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->TriggerButton(0, GamepadButtonType::LB);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->TriggerButton(0, GamepadButtonType::X);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->TriggerButton(0, GamepadButtonType::Y);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->TriggerButton(0, GamepadButtonType::RightStick);
	default:
		return false;
	}
}

//=============================================================================
// ボタン入力取得(リリース)
//=============================================================================
bool PlayerPadInput::IsReleased(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::RT);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::A);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::LT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::RB);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::LB);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::X);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::Y);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->ReleaseButton(0, GamepadButtonType::RightStick);
	default:
		return false;
	}
}